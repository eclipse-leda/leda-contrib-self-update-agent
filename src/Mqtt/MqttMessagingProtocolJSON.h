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

#ifndef SDV_SUA_MQTTMESSAGINGPROTOCOLJSON_H
#define SDV_SUA_MQTTMESSAGINGPROTOCOLJSON_H

#include "IMqttMessagingProtocol.h"

namespace sua {

    class MqttMessagingProtocolJSON : public IMqttMessagingProtocol {
    public:
        Command readCommand(const std::string & input) override;

        DesiredState readDesiredState(const std::string & input) override;

        DesiredState readCurrentStateRequest(const std::string & input) override;

        std::string createMessage(const class Context& ctx, MqttMessage message_type, const std::string& message = "") override;

    protected:
        virtual uint64_t epochTime() const;

        std::string writeFeedbackWithoutPayload(const DesiredState & desiredState,
                const std::string & state, const std::string & stateMessage) const;

        std::string writeFeedbackWithPayload(const DesiredState & desiredState,
                const std::string & state, const std::string & stateMessage,
                const std::string & status, const std::string & statusMessage,
                const std::string & customStatusMessage, int progress) const;

        std::string writeSystemVersionWithoutActivityId(const std::string & version);
        std::string writeSystemVersionWithActivityId(const std::string & version, const std::string & activityId);
    };

} // namespace sua

#endif
