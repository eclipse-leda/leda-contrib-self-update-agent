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

#include "FSM/States/Installing.h"
#include "FSM/FSM.h"
#include "FotaEvent.h"
#include "Install/Installer.h"
#include "Context.h"
#include "Logger.h"

namespace sua {

    Installing::Installing()
        : State("Installing")
    { }

    void Installing::onEnter(Context& ctx)
    {
        ctx.desiredState.installProgressPercentage = 0;
        ctx.stateMachine->handleEvent(FotaEvent::InstallStart);
    }

    FotaEvent Installing::body(Context& ctx)
    {
        subscribe(Installer::EVENT_INSTALLING, [this, &ctx](const std::map<std::string, std::string>& payload) {
            const auto percentage = std::stoi(payload.at("percentage"));
            Logger::info("RAUC install progress: {}", percentage);
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "installing");
        });

        const auto result = ctx.installerAgent->installBundle(ctx.updatesDirectory + "/temp_file");

        if(result == TechCode::OK) {
            Logger::info("RAUC install completed");
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "installed");
            return FotaEvent::InstallCompleted;
        }

        if(result == TechCode::InstallationFailed) {
            Logger::error("RAUC install failed");
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "installFailed");
            return FotaEvent::InstallFailed;
        }

        return FotaEvent::NotUsed;
    }

} // namespace sua
