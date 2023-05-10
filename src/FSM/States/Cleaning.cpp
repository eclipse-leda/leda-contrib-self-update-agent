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

#include "FSM/States/Cleaning.h"
#include "Context.h"
#include "Logger.h"

namespace sua {

    Cleaning::Cleaning()
        : Connected("Cleaning")
    { }

    void Cleaning::onEnter(Context& ctx)
    {
        const auto path   = ctx.updatesDirectory + ctx.tempFileName;
        const auto result = remove(path.c_str());

        if(result != 0) {
            Logger::error("Failed to remove temporary bundle file: '{}', reason: '{}'", path, strerror(errno));
        }

        send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Cleaned);

        if(ctx.desiredState.actionStatus == "UPDATE_SUCCESS") {
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Complete);
        } else {
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Incomplete);
        }

        ctx.stateMachine->handleEvent(FotaEvent::Waiting);
    }

} // namespace sua
