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
#include "version.h"

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
        DesiredState s;
        std::stringstream ss(input);
        nlohmann::json json = nlohmann::json::parse(ss);

        s.activityId = json.at("activityId");
        if(s.activityId.empty()) {
            throw std::logic_error("mandatory field 'activityId' is empty");
        }

        auto find_by_key_value = [] (const nlohmann::json & j, const std::string & key, const std::string & value) -> nlohmann::json::const_iterator {
            return std::find_if(j.begin(), j.end(),
                [&key, &value] (const nlohmann::json & item) {
                    return item.at(key) == value;
                }
            );
        };

        const auto & payload = json.at("payload");
        const auto & domains = payload.at("domains");
        const auto domain_it = find_by_key_value(domains, "id", "self-update");
        if(domain_it == domains.end()) {
            throw std::runtime_error("mandatory domain with 'id'='self-update' is missing");
        }

        const auto & components = domain_it->at("components");
        const auto component_it = find_by_key_value(components, "id", "os-image");
        if(component_it == components.end()) {
            throw std::runtime_error("mandatory component with 'id'='os-image' is missing");
        }

        const auto & configs = component_it->at("config");
        const auto config_it = find_by_key_value(configs, "key", "image");
        if(config_it == configs.end()) {
            throw std::runtime_error("mandatory config entry with bundle url is missing");
        }

        s.bundleVersion     = component_it->at("version");
        s.bundleDownloadUrl = config_it->at("value");

        if(s.bundleVersion.empty()) {
            throw std::runtime_error("mandatory bundle version is empty");
        }

        if(s.bundleDownloadUrl.empty()) {
            throw std::runtime_error("mandatory bundle download url is empty");
        }

        return s;
    }

    DesiredState MqttMessagingProtocolJSON::readCurrentStateRequest(const std::string & input)
    {
        DesiredState s;
        std::stringstream ss(input);

        auto json = nlohmann::json::parse(ss);
        if(!json.contains("activityId")) {
            throw std::logic_error("mandatory field 'activityId' is missing");
        }

        s.activityId = json.at("activityId");

        if(s.activityId.empty()) {
            throw std::logic_error("mandatory field 'activityId' is empty");
        }

        return s;
    }

    std::string MqttMessagingProtocolJSON::createMessage(const class Context& ctx, const std::string& name, const std::string& message)
    {
        if(name == "systemVersion") {
            if(ctx.desiredState.activityId.empty()) {
                return writeSystemVersionWithoutActivityId(ctx.currentState.version);
            } else {
                return writeSystemVersionWithActivityId(ctx.currentState.version, ctx.desiredState.activityId);
            }
        }

        if(name == "identifying") {
            return writeFeedbackWithoutPayload(ctx.desiredState, "IDENTIFYING",
                "Self-update agent has received new desired state request and is evaluating it.");
        }

        if(name == "identified") {
            return writeFeedbackWithoutPayload(ctx.desiredState,
                "IDENTIFIED", "Self-update agent is about to perform an OS image update.");
        }

        if(name == "identificationFailed") {
            return writeFeedbackWithoutPayload(ctx.desiredState,
                "IDENTIFICATION_FAILED", message);
        }

        if(name == "skipped") {
            return writeFeedbackWithoutPayload(ctx.desiredState, "COMPLETED",
                "Current OS image is equal to the target one from desired state.");
        }

        if(name == "rejected") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "INCOMPLETE", "Update rejected.",
                "UPDATE_FAILURE", "Bundle version does not match version in desired state request.",
                message, 0);
        }

        if(name == "downloading") {
            const double mbytes = static_cast<double>(ctx.desiredState.downloadBytesTotal) / 1024.0 / 1024.0;

            return writeFeedbackWithPayload(ctx.desiredState,
                "RUNNING", "Self-update agent is performing an OS image update.",
                "DOWNLOADING", fmt::format("Downloading {:.{}f} MiB...", mbytes, 1),
                message, ctx.desiredState.downloadProgressPercentage);
        }

        if(name == "downloaded") {
            double mbytes = static_cast<double>(ctx.desiredState.downloadBytesTotal) / 1024.0 / 1024.0;

            return writeFeedbackWithPayload(ctx.desiredState,
                "RUNNING", "Self-update agent is performing an OS image update.",
                "DOWNLOAD_SUCCESS", fmt::format("Downloaded {:.{}f} MiB...", mbytes, 1),
                message, 100);
        }

        if(name == "downloadFailed") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "INCOMPLETE", "Download failed.",
                "UPDATE_FAILURE", "Download failed.",
                message, ctx.desiredState.downloadProgressPercentage);
        }

        if(name == "installing") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "RUNNING", "Self-update agent is performing an OS image update.",
                "UPDATING", "RAUC install...",
                message, ctx.desiredState.installProgressPercentage);
        }

        if(name == "installed") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "COMPLETED", "Self-update completed, reboot required.",
                "UPDATE_SUCCESS", "Writing partition completed, reboot required.",
                message, 100);
        }

        if(name == "installFailed") {
            return writeFeedbackWithPayload(ctx.desiredState,
                "INCOMPLETE", "Install failed.",
                "UPDATE_FAILURE", "Writing partition failed.",
                message, ctx.desiredState.installProgressPercentage);
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
                const std::string & customStatusMessage,
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
        
        return fmt::format(tpl, desiredState.activityId, epochTime(),
                state, stateMessage,
                desiredState.bundleVersion,
                status, progress,
                (customStatusMessage.empty() ? statusMessage : customStatusMessage)
                );
    }

    std::string MqttMessagingProtocolJSON::writeSystemVersionWithoutActivityId(const std::string & version)
    {
        // clang-format off
        const std::string tpl = jsonTemplate(R"(
            {
                "timestamp": {},
                "payload": {
                    "softwareNodes": [
                        {
                            "id": "self-update-agent",
                            "version": "build-{}",
                            "name": "OTA NG Self Update Agent",
                            "type": "APPLICATION"
                        },
                        {
                            "id": "self-update:leda-deviceimage",
                            "version": "{}",
                            "name": "Official Leda device image",
                            "type": "IMAGE"
                        }
                    ],
                    "hardwareNodes": [],
                    "associations": [
                        {
                            "sourceId": "self-update-agent",
                            "targetId": "self-update:leda-deviceimage"
                        }
                    ]
                }
            }
        )");
        // clang-format on

        return fmt::format(tpl, epochTime(), SUA_BUILD_NUMBER, version);
    }

    std::string MqttMessagingProtocolJSON::writeSystemVersionWithActivityId(const std::string & version, const std::string & activityId)
    {
        // clang-format off
        const std::string tpl = jsonTemplate(R"(
            {
                "activityId": "{}",
                "timestamp": {},
                "payload": {
                    "softwareNodes": [
                        {
                            "id": "self-update-agent",
                            "version": "build-{}",
                            "name": "OTA NG Self Update Agent",
                            "type": "APPLICATION"
                        },
                        {
                            "id": "self-update:leda-deviceimage",
                            "version": "{}",
                            "name": "Official Leda device image",
                            "type": "IMAGE"
                        }
                    ],
                    "hardwareNodes": [],
                    "associations": [
                        {
                            "sourceId": "self-update-agent",
                            "targetId": "self-update:leda-deviceimage"
                        }
                    ]
                }
            }
        )");
        // clang-format on

        return fmt::format(tpl, activityId, epochTime(), SUA_BUILD_NUMBER, version);
    }

} // namespace sua
