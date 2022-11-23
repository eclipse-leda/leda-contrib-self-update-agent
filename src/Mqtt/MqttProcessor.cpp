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
#include "Logger.h"
#include "Mqtt/MqttListener.h"
#include "Mqtt/MqttMessageDeserializer.h"
#include "Mqtt/MqttMessageSerializer.h"
#include "Patterns/Dispatcher.h"

#include "yaml/Yaml.hpp"

#include <string>

namespace {
    int QUALITY     = 1;
    int MAX_RETRIES = 50;

    std::string TOPIC_START         = "selfupdate/desiredstate";
    std::string TOPIC_STATE         = "selfupdate/desiredstatefeedback";
    std::string TOPIC_CURRENT_STATE = "selfupdate/currentstate";

    class MqttActionListener : public virtual mqtt::iaction_listener {
    public:
        MqttActionListener(const std::string& name)
            : _name(name)
        { }

    private:
        void on_failure(const mqtt::token& token) override { }

        void on_success(const mqtt::token& token) override { }

    private:
        std::string _name;
    };

    class MqttCallback
        : public virtual mqtt::callback
        , public virtual mqtt::iaction_listener {
    public:
        MqttCallback(mqtt::async_client&    client,
                     mqtt::connect_options& options,
                     sua::MqttListener*     listener)
            : _retries(0)
            , _mqttClient(client)
            , _options(options)
            , _actionListener("Subscription")
            , _listener(listener)
        { }

    private:
        void reconnect()
        {
            sua::Logger::info("Reconnect attempt to MQTT broker");
            sua::Dispatcher::instance().dispatch(sua::MqttProcessor::EVENT_CONNECTING, "");
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
            sua::Dispatcher::instance().dispatch(sua::MqttProcessor::EVENT_DISCONNECTED, "");
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
            _mqttClient.subscribe(TOPIC_START, QUALITY, nullptr, _actionListener);
            sua::Dispatcher::instance().dispatch(sua::MqttProcessor::EVENT_CONNECTED, "");
        }

        void connection_lost(const std::string& cause) override
        {
            sua::Logger::trace("MqttCallback::connection_lost");
            sua::Dispatcher::instance().dispatch(sua::MqttProcessor::EVENT_DISCONNECTED, "");
            _retries = 0;
            reconnect();
        }

        void message_arrived(mqtt::const_message_ptr msg) override
        {
            if(msg->get_topic() == TOPIC_START) {
                sua::MessageStart            m;
                sua::MqttMessageDeserializer d;
                d.deserialize(msg->get_payload_str(), m);
                _listener->handle(m);
                return;
            }
        }

        void delivery_complete(mqtt::delivery_token_ptr token) override { }

    private:
        int                    _retries;
        mqtt::async_client&    _mqttClient;
        mqtt::connect_options& _options;
        MqttActionListener     _actionListener;
        sua::MqttListener*     _listener = nullptr;
    };
} // namespace

namespace sua {
    const std::string MqttProcessor::EVENT_CONNECTING   = "Mqtt/Connecting";
    const std::string MqttProcessor::EVENT_CONNECTED    = "Mqtt/Connected";
    const std::string MqttProcessor::EVENT_DISCONNECTED = "Mqtt/Disconnected";

    MqttProcessor::MqttProcessor(MqttConfiguration configuration, MqttListener* listener)
        : _client(configuration.brokerHost, _clientId)
        , _listener(listener)
    {
    }

    MqttProcessor::~MqttProcessor() { }

    void MqttProcessor::start()
    {
        Logger::info("Connecting to Mqtt broker, port: 1883, host: {}", _client.get_server_uri());
        sua::Dispatcher::instance().dispatch(sua::MqttProcessor::EVENT_CONNECTING, "");
        mqtt::connect_options options;
        options.set_clean_session(false);

        static MqttCallback callback(_client, options, _listener);
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
        sua::Dispatcher::instance().dispatch(sua::MqttProcessor::EVENT_DISCONNECTED, "");
        try {
            _client.disconnect()->wait();
        } catch(const mqtt::exception& e) {
            std::cerr << e << std::endl;
        }
    }

    void MqttProcessor::sendState(const MessageState& state)
    {
        Logger::info("Sending state to MQTT broker");

        if(!_client.is_connected()) {
            Logger::error("Not connected to MQTT broker, will not send state");
            return;
        }

        MqttMessageSerializer s;
        mqtt::message_ptr     message = mqtt::make_message(TOPIC_STATE, s.serialize(state));
        message->set_qos(QUALITY);
        _client.publish(message);
    }

    void MqttProcessor::sendCurrentState(const MessageCurrentState& state)
    {
        Logger::info("Sending current state to MQTT broker");

        if(!_client.is_connected()) {
            Logger::error("Not connected to MQTT broker, will not send current state");
            return;
        }

        MqttMessageSerializer s;
        mqtt::message_ptr     message = mqtt::make_message(TOPIC_CURRENT_STATE, s.serialize(state));
        message->set_qos(QUALITY);
        message->set_retained(true);
        _client.publish(message);
    }
} // namespace sua
