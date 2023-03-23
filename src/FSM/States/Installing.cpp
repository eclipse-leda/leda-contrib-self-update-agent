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
            ctx.desiredState.installProgressPercentage = std::stoi(payload.at("percentage"));
            Logger::info("Install progress: {}%", ctx.desiredState.installProgressPercentage);
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "installing");
        });

        std::string install_input;
        if (true == ctx.downloadMode)
        {
            install_input = ctx.updatesDirectory + "/temp_file";
        }
        else
        {
            // installation streamed via download URL
            install_input = ctx.desiredState.bundleDownloadUrl;
        }

        auto installer = Installer(ctx.installerAgent);
        const auto result = installer.start(install_input);

        if(result == TechCode::OK) {
            Logger::info("Installation completed");
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "installed");
            return FotaEvent::InstallCompleted;
        }

        const auto lastError = ctx.installerAgent->getLastError();
        Logger::error("Installation failed: {}", lastError);

        // for download mode transit to fail state
        if (true == ctx.downloadMode) {
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "installFailed", lastError);
            return FotaEvent::InstallFailed;
        }

        // for stream mode start again in download mode
        Logger::info("Trying normal download mode as fallback");
        send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "installFailedFallback", lastError);
        ctx.downloadMode = true;
        return FotaEvent::InstallFailedFallback;
    }

} // namespace sua
