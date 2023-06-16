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

#ifndef SDV_SUA_MQTTPROCESSOR_H
#define SDV_SUA_MQTTPROCESSOR_H

#include "Mqtt/IMqttProcessor.h"
#include "Mqtt/MqttConfiguration.h"

#include "mqtt/async_client.h"

namespace sua {

    class MqttProcessor : public IMqttProcessor {
    public:
        MqttProcessor(class Context & context);
        ~MqttProcessor() = default;

        void start(const MqttConfiguration & configuration) override;
        void stop() override;

        void send(const std::string& topic, MqttMessage message_type, const std::string& message = "", bool retained = false) override;

    private:
        Context & _context;

        MqttConfiguration _config;

        std::string        _clientId = "sua";
        mqtt::async_client _client;
    };
} // namespace sua

#endif
