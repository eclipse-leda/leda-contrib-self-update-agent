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

#include "MqttMessagingProtocolJSON.h"
#include "Context.h"

#include "spdlog/fmt/fmt.h"
#include "nlohmann/json.hpp"

#include <regex>
#include <iostream>
#include <chrono>

namespace {

    std::string jsonTemplate(std::string tpl)
    {
        // required because lib-fmt expects curly brackets escaped as {{ and }}
        // to properly handle placeholders 3 steps are done:
        //   { -> {{
        //   } -> }}
        // this makes placeholder {} look like {{}}
        //   {{}} -> {}

        // escape opening
        tpl = std::regex_replace(tpl, std::regex("\\{"), "{{");
        // espace closing
        tpl = std::regex_replace(tpl, std::regex("\\}"), "}}");
        // unescape placeholder
        tpl = std::regex_replace(tpl, std::regex("\\{\\{\\}\\}"), "{}");

        return tpl;
    }

}

namespace sua {

    uint64_t MqttMessagingProtocolJSON::epochTime() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    DesiredState MqttMessagingProtocolJSON::readDesiredState(const std::string & input)
    {
        // clang-format off
        DesiredState s;
        std::stringstream ss(input);
        nlohmann::json json = nlohmann::json::parse(ss);
        s.activityId        = json["activityId"];
        s.bundleVersion     = json["payload"]["domains"][0]["components"][0]["version"];
        s.bundleDownloadUrl = json["payload"]["domains"][0]["components"][0]["config"][0]["value"];
        return s;
        // clang-format on
    }

    DesiredState MqttMessagingProtocolJSON::readCurrentStateRequest(const std::string & input)
    {
        // clang-format off
        DesiredState s;
        std::stringstream ss(input);
        nlohmann::json json = nlohmann::json::parse(ss);
        s.activityId        = json["activityId"];
        return s;
        // clang-format on
    }

    std::string MqttMessagingProtocolJSON::createMessage(const class Context& ctx, const std::string& name)
    {
        if(name == "systemVersion") {
            // clang-format off
            const std::string tpl = jsonTemplate(R"(
                {
                    "timestamp": {},
                    "payload": {
                        "domains": [
                            {
                                "id": "self-update",
                                "components": [
                                    {
                                        "id": "os-image",
                                        "version": "{}"
                                    }
                                ]
                            }
                        ]
                    }
                }
            )");
            // clang-format on

            return fmt::format(tpl, epochTime(), ctx.currentState.version);
        }

        if(name == "identifying") {
            return writeFeedbackWithoutPayload(ctx.desiredState, "IDENTIFYING",
                "Self-update agent has received new desired state request and is evaluating it.");
        }

        if(name == "identified") {
            return writeFeedbackWithoutPayload(ctx.desiredState,
                "IDENTIFIED", "Self-update agent is about to perform an OS image update.");
        }

        if(name == "skipped") {
            return writeFeedbackWithoutPayload(ctx.desiredState, "COMPLETE",
                "Current OS image is equal to the target one from desired state.");
        }

        if(name == "rejected") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "INCOMPLETE", "Update rejected.",
                "UPDATE_FAILURE", "Bundle version does not match version in desired state request.",
                0);
        }

        if(name == "downloading") {
            const double mbytes = static_cast<double>(ctx.desiredState.downloadBytesTotal) / 1024.0 / 1024.0;

            return writeFeedbackWithPayload(ctx.desiredState,
                "RUNNING", "Self-update agent is performing an OS image update.",
                "DOWNLOADING", fmt::format("Downloading {:.{}f} MiB...", mbytes, 1),
                ctx.desiredState.downloadProgressPercentage);
        }

        if(name == "downloaded") {
            double mbytes = static_cast<double>(ctx.desiredState.downloadBytesTotal) / 1024.0 / 1024.0;

            return writeFeedbackWithPayload(ctx.desiredState,
                "RUNNING", "Self-update agent is performing an OS image update.",
                "DOWNLOAD_SUCCESS", fmt::format("Downloaded {:.{}f} MiB...", mbytes, 1),
                100);
        }

        if(name == "downloadFailed") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "INCOMPLETE", "Download failed.",
                "UPDATE_FAILURE", "Download failed.",
                ctx.desiredState.downloadProgressPercentage);
        }

        if(name == "installing") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "RUNNING", "Self-update agent is performing an OS image update.",
                "UPDATING", "RAUC install...",
                ctx.desiredState.installProgressPercentage);
        }

        if(name == "installed") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "COMPLETE", "Self-update completed, reboot required.",
                "UPDATE_SUCCESS", "Writing partition completed, reboot required.",
                100);
        }

        if(name == "installFailed") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "INCOMPLETE", "Install failed.",
                "UPDATE_FAILURE", "Writing partition failed.",
                ctx.desiredState.installProgressPercentage);
        }

        if(name == "currentState") {
            return "";
        }

        throw std::logic_error(fmt::format("Unknown message type '{}'", name));
    }

    std::string MqttMessagingProtocolJSON::writeFeedbackWithoutPayload(const DesiredState & desiredState,
                const std::string & state, const std::string & stateMessage) const
    {
        // clang-format off
        const std::string tpl = jsonTemplate(R"(
            {
                "activityId": "{}",
                "timestamp": {},
                "payload": {
                    "status": "{}",
                    "message": "{}",
                    "actions": []
                }
            }
        )");
        // clang-format on

        return fmt::format(tpl, desiredState.activityId, epochTime(), state, stateMessage);
    }

    std::string MqttMessagingProtocolJSON::writeFeedbackWithPayload(const DesiredState & desiredState,
                const std::string & state, const std::string & stateMessage,
                const std::string & status, const std::string & statusMessage,
                int progress) const
    {
        // clang-format off
        const std::string tpl = jsonTemplate(R"(
            {
                "activityId": "{}",
                "timestamp": {},
                "payload": {
                    "status": "{}",
                    "message": "{}",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "{}"
                            },
                            "status": "{}",
                            "progress": {},
                            "message": "{}"
                        }
                    ]
                }
            }
        )");
        // clang-format on
        
        return fmt::format(tpl, desiredState.activityId, epochTime(), state, stateMessage, desiredState.bundleVersion, status, progress, statusMessage);
    }

} // namespace sua
