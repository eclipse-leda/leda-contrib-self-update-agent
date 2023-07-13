//    Copyright 2023 Contributors to the Eclipse Foundation
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
//    SPDX-License-Identifier: Apache-2.0

#include "Mqtt/MqttProcessor.h"
#include "Mqtt/MqttMessage.h"
#include "Context.h"
#include "FotaEvent.h"
#include "Logger.h"

#include "nlohmann/json.hpp"

#include <string>
#include <chrono>

using namespace std::chrono_literals;

namespace {

    int QUALITY     = 1;
    int MAX_RETRIES = 50;

    class MqttActionListener : public virtual mqtt::iaction_listener {
    public:
        MqttActionListener()
        { }

    private:
        void on_failure(const mqtt::token&) override { }

        void on_success(const mqtt::token&) override { }
    };

    class MqttCallback
        : public virtual mqtt::callback
        , public virtual mqtt::iaction_listener {
    public:
        MqttCallback(mqtt::async_client    & client,
                     mqtt::connect_options & options,
                     sua::Context          & context)
            : _retries(0)
            , _mqttClient(client)
            , _options(options)
            , _context(context)
        { }

    private:
        void reconnect()
        {
            sua::Logger::info("Reconnect attempt to MQTT broker");
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            try {
                sua::Logger::info("Connect attempt to MQTT broker");
                _mqttClient.reconnect();
            } catch(const mqtt::exception& exc) {
                sua::Logger::error("Connect attempt failed: {}", exc.what());
                exit(1);
            }
        }

        void on_failure(const mqtt::token& tok) override
        {
            sua::Logger::trace("MqttCallback::on_failure, code = {}", tok.get_reason_code());
            _context.stateMachine->handleEvent(sua::FotaEvent::ConnectivityLost);
            if(++_retries > MAX_RETRIES) {
                sua::Logger::error("Max number of reconnect attempts reached, will exit");
                exit(1);
            }
            reconnect();
        }

        void on_success(const mqtt::token& tok) override
        {
            sua::Logger::trace("MqttCallback::on_success");
        }

        void connected(const std::string& cause) override
        {
            sua::Logger::trace("MqttCallback::connected");
            _mqttClient.subscribe(sua::IMqttProcessor::TOPIC_IDENTIFY, QUALITY, nullptr, _actionListener);
            _mqttClient.subscribe(sua::IMqttProcessor::TOPIC_STATE_GET, QUALITY, nullptr, _actionListener);
            _mqttClient.subscribe(sua::IMqttProcessor::TOPIC_COMMAND, QUALITY, nullptr, _actionListener);
            _context.stateMachine->handleEvent(sua::FotaEvent::ConnectivityEstablished);
        }

        void connection_lost(const std::string& cause) override
        {
            sua::Logger::trace("MqttCallback::connection_lost");
            _context.stateMachine->handleEvent(sua::FotaEvent::ConnectivityLost);
            _retries = 0;
            reconnect();
        }

        void message_arrived(mqtt::const_message_ptr msg) override
        {
            auto & ctx = _context;
            auto proto = ctx.messagingProtocol;
            auto mqtt  = ctx.mqttProcessor;

            const auto message = msg->get_payload_str();
            const auto topic   = msg->get_topic();

            sua::Logger::trace("Received message via topic '{}'", topic);

            try {
                if(topic == sua::IMqttProcessor::TOPIC_IDENTIFY) {
                    ctx.desiredState = proto->readDesiredState(message);
                    ctx.stateMachine->handleEvent(sua::FotaEvent::Identify);
                } else if(topic == sua::IMqttProcessor::TOPIC_STATE_GET) {
                    ctx.desiredState = proto->readCurrentStateRequest(message);
                    ctx.stateMachine->handleEvent(sua::FotaEvent::GetCurrentState);
                } else if(topic == sua::IMqttProcessor::TOPIC_COMMAND) {
                    sua::Command c = proto->readCommand(message);
                    ctx.desiredState.activityId = c.activityId;
                    ctx.stateMachine->handleEvent(c.event);
                } else {
                    sua::Logger::error("Invalid topic: '{}'", topic);
                }
            } catch(const nlohmann::json::parse_error & e) {
                // catch here if request is not a valid json (yaml or xml for example)
                sua::Logger::error("Unable to parse desired state request (not a valid json?): '{}'", e.what());
            } catch(const nlohmann::json::exception& e) {
                // catch here if request is incomplete (missing field)
                // result is lost, extract activityId again and reply IdentificationFailed
                ctx.desiredState.activityId = nlohmann::json::parse(message).at("activityId");
                sua::Logger::error("Incomplete desired state request, unable to identify (incomplete json?): '{}'", e.what());
                mqtt->send(sua::IMqttProcessor::TOPIC_FEEDBACK, sua::MqttMessage::Identifying);
                mqtt->send(sua::IMqttProcessor::TOPIC_FEEDBACK, sua::MqttMessage::IdentificationFailed, e.what());
            } catch(const std::logic_error & e) {
                // catch here if request is valid json but activityId is empty
                sua::Logger::error("Invalid desired state request: '{}'", e.what());
            } catch(const std::runtime_error & e) {
                // catch here if request is incomplete (empty field)
                // result is lost, extract activityId again and reply IdentificationFailed
                ctx.desiredState.activityId = nlohmann::json::parse(message).at("activityId");
                sua::Logger::error("Malformed desired state request, unable to identify: '{}'", e.what());
                mqtt->send(sua::IMqttProcessor::TOPIC_FEEDBACK, sua::MqttMessage::Identifying);
                mqtt->send(sua::IMqttProcessor::TOPIC_FEEDBACK, sua::MqttMessage::IdentificationFailed, e.what());
            }
        }

        void delivery_complete(mqtt::delivery_token_ptr token) override { }

    private:
        int                     _retries;
        mqtt::async_client&     _mqttClient;
        mqtt::connect_options&  _options;
        MqttActionListener      _actionListener;
        sua::Context          & _context;
    };

} // namespace

namespace sua {

    MqttProcessor::MqttProcessor(Context & context)
        : _context(context)
        , _client("", _clientId)
    { }

    void MqttProcessor::start(const MqttConfiguration & configuration)
    {
        _config = configuration;

        Logger::info("MQTT broker address: '{}:{}'", _config.brokerHost, _config.brokerPort);

        static mqtt::connect_options options;
        options.set_clean_session(false);
        options.set_servers(std::make_shared<mqtt::string_collection>(
            fmt::format("{}:{}", _config.brokerHost, _config.brokerPort)
        ));
        options.set_automatic_reconnect(true);
        options.set_connect_timeout(5000ms);

        static MqttCallback callback(_client, options, _context);
        _client.set_callback(callback);

        bool connected = false;
        while(!connected) {
            try {
                connected = true;
                _client.connect(options, nullptr, callback);
            } catch(const mqtt::exception& e) {
                connected = false;
                Logger::error("Unable to connect to MQTT server: {}, error: {}",
                              _client.get_server_uri(),
                              e.what());
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            }
        }
    }

    void MqttProcessor::stop()
    {
        Logger::info("Disconnecting from MQTT broker");
        _context.stateMachine->handleEvent(FotaEvent::ConnectivityLost);
        try {
            _client.disconnect()->wait();
        } catch(const mqtt::exception& e) {
            std::cerr << e << std::endl;
        }
    }

    void MqttProcessor::send(const std::string& topic, MqttMessage message_type, const std::string& message, bool retained)
    {
        const auto content = _context.messagingProtocol->createMessage(_context, message_type, message);

        auto mqtt_message = mqtt::make_message(topic, content);
        mqtt_message->set_qos(QUALITY);
        mqtt_message->set_retained(retained);
        try {
            _client.publish(mqtt_message);
        } catch(const mqtt::exception& e) {
            Logger::error("Publish to topic '{}' failed: '{}'", topic, e.what());
        }

        if(!_client.is_connected() || !_client.get_pending_delivery_tokens().empty()) {
            _client.reconnect();
        }
    }

} // namespace sua
