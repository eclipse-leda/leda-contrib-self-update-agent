//    Copyright 2022 Contributors to the Eclipse Foundation
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
#include "Context.h"
#include "FotaEvent.h"
#include "Logger.h"

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
            _mqttClient.subscribe(sua::IMqttProcessor::TOPIC_START, QUALITY, nullptr, _actionListener);
            _mqttClient.subscribe(sua::IMqttProcessor::TOPIC_STATE_GET, QUALITY, nullptr, _actionListener);
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
            if(msg->get_topic() == sua::IMqttProcessor::TOPIC_START) {
                _context.desiredState = _context.messagingProtocol->readDesiredState(msg->get_payload_str());
                _context.stateMachine->handleEvent(sua::FotaEvent::Start);
            } if(msg->get_topic() == sua::IMqttProcessor::TOPIC_STATE_GET) {
                _context.desiredState = _context.messagingProtocol->readCurrentStateRequest(msg->get_payload_str());
                _context.stateMachine->handleEvent(sua::FotaEvent::GetCurrentState);
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

    MqttProcessor::MqttProcessor(const MqttConfiguration & configuration, Context & context)
        : _context(context)
        , _config(configuration)
        , _client(configuration.brokerHost, _clientId)
    { }

    void MqttProcessor::start()
    {
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

    void MqttProcessor::send(const std::string& topic, const std::string& content, bool retained)
    {
        if(content.empty()) {
            return;
        }

        auto message = mqtt::make_message(topic, content);
        message->set_qos(QUALITY);
        message->set_retained(retained);
        _client.publish(message);

        if(!_client.is_connected() || !_client.get_pending_delivery_tokens().empty()) {
            _client.reconnect();
        }
    }

} // namespace sua
