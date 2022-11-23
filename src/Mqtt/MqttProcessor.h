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

#ifndef SDV_SUA_MQTTPROCESSOR_H
#define SDV_SUA_MQTTPROCESSOR_H

#include "Mqtt/MqttConfiguration.h"
#include "PayloadMessages.h"

#include "mqtt/async_client.h"

namespace sua {
    class MqttListener;

    class MqttProcessor {
    public:
        static const std::string EVENT_CONNECTING;
        static const std::string EVENT_CONNECTED;
        static const std::string EVENT_DISCONNECTED;

        MqttProcessor(MqttConfiguration configuration, MqttListener* listener);
        ~MqttProcessor();

        void start();
        void stop();

        void sendState(const MessageState& message);
        void sendCurrentState(const MessageCurrentState& message);

    private:
        std::string        _clientId = "sua";
        mqtt::async_client _client;
        MqttListener*      _listener = nullptr;
    };
} // namespace sua

#endif
