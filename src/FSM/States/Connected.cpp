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

#include "FSM/States/Connected.h"
#include "FSM/FSM.h"
#include "Context.h"
#include "FotaEvent.h"
#include "Logger.h"

namespace sua {

    Connected::Connected()
        : Connected("Connected")
    { }

    Connected::Connected(const std::string& name)
        : State(name)
    { }

    void Connected::onEnter(Context& ctx)
    {
        send(ctx, IMqttProcessor::TOPIC_STATE, MqttMessage::SystemVersion, true);
    }

    FotaEvent Connected::body(Context& ctx)
    {
        Logger::info("System version, installed: '{}'", ctx.currentState.version);

        send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Identifying);

        if(ctx.bundleChecker->isUpdateBundleVersionDifferent(ctx.desiredState.bundleVersion,
                                                             ctx.installerAgent)) {
            Logger::info("Bundle file and slot versions differ.");
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Identified);
            return FotaEvent::BundleVersionOK;
        }

        Logger::info("Bundle version unchanged");
        send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Skipped);
        return FotaEvent::BundleVersionUnchanged;
    }

} // namespace sua
