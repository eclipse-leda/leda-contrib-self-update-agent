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
    }

    FotaEvent Installing::body(Context& ctx)
    {
        subscribe(Installer::EVENT_INSTALLING, [this, &ctx](const std::map<std::string, std::string>& payload) {
            ctx.desiredState.installProgressPercentage = std::stoi(payload.at("percentage"));
            Logger::info("Install progress: {}%", ctx.desiredState.installProgressPercentage);
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Installing);
        });

        std::string install_input;
        if (true == ctx.downloadMode)
        {
            install_input = ctx.updatesDirectory + ctx.tempFileName;
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::VersionChecking);

            if(ctx.bundleChecker->isBundleVersionConsistent(
                   ctx.desiredState.bundleVersion, ctx.installerAgent, install_input)) {
                Logger::info("Downloaded bundle version matches spec.");
            } else {
                Logger::info("Downloaded bundle version does not match spec.");
                ctx.desiredState.actionStatus = "UPDATE_FAILURE";
                ctx.desiredState.actionMessage =
                    "Bundle version does not match version in desired state request.";
                send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Rejected);
                return FotaEvent::BundleVersionInconsistent;
            }
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
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::Installed);

            if(ctx.fallbackMode) {
                ctx.downloadMode = false;
                ctx.fallbackMode = false;
            }

            return FotaEvent::InstallCompleted;
        }

        const auto lastError = ctx.installerAgent->getLastError();
        Logger::error("Installation failed: {}", lastError);

        // for download mode transit to fail state
        if (true == ctx.downloadMode) {
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::InstallFailed, lastError);

            if(ctx.fallbackMode) {
                ctx.downloadMode = false;
                ctx.fallbackMode = false;
            }

            ctx.desiredState.actionStatus  = "UPDATE_FAILURE";
            ctx.desiredState.actionMessage = lastError;

            return FotaEvent::InstallFailed;
        }

        // for stream mode start again in download mode
        Logger::info("Trying normal download mode as fallback");
        send(ctx, IMqttProcessor::TOPIC_FEEDBACK, MqttMessage::InstallFailedFallback, lastError);
        ctx.downloadMode = true;
        ctx.fallbackMode = true;
        return FotaEvent::InstallFailedFallback;
    }

} // namespace sua
