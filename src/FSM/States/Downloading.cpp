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

#include "FSM/States/Downloading.h"
#include "Download/Downloader.h"
#include "Mqtt/IMqttMessagingProtocol.h"
#include "TechCodes.h"
#include "Context.h"
#include "FotaEvent.h"
#include "Logger.h"

namespace sua {

    Downloading::Downloading()
        : State("Downloading")
    { }

    void Downloading::onEnter(Context& ctx)
    {
        ctx.desiredState.downloadBytesTotal         = 0;
        ctx.desiredState.downloadBytesDownloaded    = 0;
        ctx.desiredState.downloadProgressPercentage = 0;
        ctx.stateMachine->handleEvent(FotaEvent::DownloadStart);
    }

    FotaEvent Downloading::body(Context& ctx)
    {
        subscribe(Downloader::EVENT_DOWNLOADING, [this, &ctx](const std::map<std::string, std::string>& payload) {
            ctx.desiredState.downloadBytesTotal         = std::stoll(payload.at("total"));
            ctx.desiredState.downloadBytesDownloaded    = std::stoll(payload.at("downloaded"));
            ctx.desiredState.downloadProgressPercentage = std::stoi(payload.at("percentage"));

            Logger::info("Download progress: {}", ctx.desiredState.downloadProgressPercentage);
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "downloading");
        });

        Logger::info("Downloading bundle: '{}'", ctx.desiredState.bundleDownloadUrl);
        const auto result = ctx.downloaderAgent->start(ctx.desiredState.bundleDownloadUrl);

        if(result == TechCode::OK) {
            Logger::info("Download progress: 100");
            send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "downloaded");

            const auto bundleVersion = ctx.installerAgent->getBundleVersion(ctx.updatesDirectory + "/temp_file");
            Logger::info("Bundle version (spec): '{}'", ctx.desiredState.bundleVersion);
            Logger::info("Bundle version (file): '{}'", bundleVersion);

            if(ctx.desiredState.bundleVersion == bundleVersion) {
                Logger::info("Bundle version matches spec");
                return FotaEvent::BundleVersionOK;
            } else {
                Logger::info("Bundle version does not match spec");
                send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "rejected");
                return FotaEvent::BundleVersionInconsistent;
            }
        }

        Logger::error("Download failed");
        send(ctx, IMqttProcessor::TOPIC_FEEDBACK, "downloadFailed");
        return FotaEvent::DownloadFailed;
    }

} // namespace sua
