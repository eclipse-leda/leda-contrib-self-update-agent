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

#ifndef SDV_SUA_MOCKMQTTMESSAGINGPROTOCOL_H
#define SDV_SUA_MOCKMQTTMESSAGINGPROTOCOL_H

#include "Mqtt/IMqttMessagingProtocol.h"
#include "Context.h"

#include "gmock/gmock.h"

class MockMqttMessagingProtocol : public sua::IMqttMessagingProtocol {
public:
    MOCK_METHOD(sua::DesiredState, readDesiredState, (const std::string & input), (override));
    MOCK_METHOD(sua::DesiredState, readCurrentStateRequest, (const std::string & input), (override));
    MOCK_METHOD(std::string, createMessage, (const sua::Context& ctx, const std::string& name, const std::string& message), (override));

    std::string native_createMessage(const sua::Context& ctx, const std::string& name, const std::string& message) {
        return sua::IMqttMessagingProtocol::createMessage(ctx, name, message);
    }
};

#endif
