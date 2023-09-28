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

#include "MqttMessagingProtocolJSON.h"
#include "MqttMessage.h"
#include "Context.h"
#include "Utils/JsonUtils.h"
#include "version.h"

#include "spdlog/fmt/fmt.h"
#include "nlohmann/json.hpp"

#include <chrono>

namespace sua {

    uint64_t MqttMessagingProtocolJSON::epochTime() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    Command MqttMessagingProtocolJSON::readCommand(const std::string & input)
    {
        Command c;
        std::stringstream ss(input);
        nlohmann::json json = nlohmann::json::parse(ss);

        c.activityId = json.at("activityId");
        if(c.activityId.empty()) {
            throw std::logic_error("Mandatory field 'activityId' is empty.");
        }

        const auto & command = json.at("payload").at("command");
        if(command == "DOWNLOAD") {
            c.event = FotaEvent::DownloadStart;
        } else if(command == "UPDATE") {
            c.event = FotaEvent::InstallStart;
        } else if(command == "ACTIVATE") {
            c.event = FotaEvent::Activate;
        } else if(command == "CLEANUP") {
            c.event = FotaEvent::Cleanup;
        } else if(command == "ROLLBACK") {
            c.event = FotaEvent::Rollback;
        } else {
            throw std::runtime_error(fmt::format("unknown command '{}'", command));
        }

        return c;
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

    std::string MqttMessagingProtocolJSON::createMessage(const class Context& ctx, MqttMessage message_type, const std::string& message)
    {
        switch(message_type) {
        case MqttMessage::SystemVersion:
            if(ctx.desiredState.activityId.empty()) {
                return writeSystemVersionWithoutActivityId(ctx.currentState.version);
            } else {
                return writeSystemVersionWithActivityId(ctx.currentState.version, ctx.desiredState.activityId);
            }
        case MqttMessage::Identifying:
            return writeFeedbackWithoutPayload(ctx.desiredState, "IDENTIFYING",
                "Self-update agent has received new desired state request and is evaluating it.");
        case MqttMessage::Identified:
            return writeFeedbackWithPayload(ctx.desiredState,
                "IDENTIFIED", "Self-update agent is about to perform an OS image update.",
                "IDENTIFIED", "Self-update agent is about to perform an OS image update.",
                message, 0);
        case MqttMessage::IdentificationFailed:
            return writeFeedbackWithoutPayload(ctx.desiredState,
                "IDENTIFICATION_FAILED", message);
        case MqttMessage::Skipped:
            return writeFeedbackWithoutPayload(ctx.desiredState, "COMPLETED",
                "Current OS image is equal to the target one from desired state.");
        case MqttMessage::Rejected:
            return writeFeedbackWithPayload(ctx.desiredState,
                "UPDATE_FAILURE", "Update rejected.",
                "UPDATE_FAILURE", "Bundle version does not match version in desired state request.",
                message, 0);
        case MqttMessage::Downloading: {
            const double mbytes = static_cast<double>(ctx.desiredState.downloadBytesTotal) / 1024.0 / 1024.0;

            return writeFeedbackWithPayload(ctx.desiredState,
                "DOWNLOADING", "Self-update agent is performing an OS image update.",
                "DOWNLOADING", fmt::format("Downloading {:.{}f} MiB...", mbytes, 1),
                message, ctx.desiredState.downloadProgressPercentage);
        }
        case MqttMessage::Downloaded: {
            double mbytes = static_cast<double>(ctx.desiredState.downloadBytesTotal) / 1024.0 / 1024.0;

            return writeFeedbackWithPayload(ctx.desiredState,
                "DOWNLOAD_SUCCESS", "Self-update agent is performing an OS image update.",
                "DOWNLOAD_SUCCESS", fmt::format("Downloaded {:.{}f} MiB...", mbytes, 1),
                message, 100);
        }
        case MqttMessage::DownloadFailed:
            return writeFeedbackWithPayload(ctx.desiredState,
                "DOWNLOAD_FAILURE", "Download failed.",
                ctx.desiredState.actionStatus, ctx.desiredState.actionMessage,
                message, ctx.desiredState.downloadProgressPercentage);
        case MqttMessage::VersionChecking:
            return writeFeedbackWithPayload(ctx.desiredState,
                "UPDATING", "Self-update agent is performing an OS image update.",
                "UPDATING", "Checking bundle version and version in desired state request.",
                message, 0);
        case MqttMessage::Installing:
            return writeFeedbackWithPayload(ctx.desiredState,
                "UPDATING", "Self-update agent is performing an OS image update.",
                "UPDATING", "RAUC install...",
                message, ctx.desiredState.installProgressPercentage);
        case MqttMessage::Installed:
            return writeFeedbackWithPayload(ctx.desiredState,
                "UPDATE_SUCCESS", "Self-update completed, reboot required.",
                "UPDATING", "Writing partition completed, reboot required.",
                message, 100);
        case MqttMessage::InstallFailed:
            return writeFeedbackWithPayload(ctx.desiredState,
                "UPDATE_FAILURE", "Install failed.",
                "UPDATE_FAILURE", "Writing partition failed.",
                message, ctx.desiredState.installProgressPercentage);
        case MqttMessage::InstallFailedFallback:
            return writeFeedbackWithPayload(ctx.desiredState,
                "RUNNING", "Self-update agent is performing an OS image update.",
                "UPDATING", "Install in streaming mode failed, trying in download mode.",
                message, 0);
        case MqttMessage::CurrentState:
            return "";
        case MqttMessage::Cleaned:
            return writeFeedbackWithPayload(ctx.desiredState,
                "CLEANUP_SUCCESS", "Self-update agent has cleaned up after itself.",
                ctx.desiredState.actionStatus, ctx.desiredState.actionMessage,
                message, 0);
        case MqttMessage::Activating:
            return writeFeedbackWithPayload(ctx.desiredState,
                "ACTIVATING", "Self-update agent is performing an OS image activation.",
                "UPDATING", "Self-update agent is performing an OS image activation.",
                message, 0);
        case MqttMessage::Activated:
            return writeFeedbackWithPayload(ctx.desiredState,
                "ACTIVATION_SUCCESS", "Self-update agent has activated the new OS image.",
                "UPDATED", "Self-update agent has activated the new OS image.",
                message, 0);
        case MqttMessage::ActivationFailed:
            return writeFeedbackWithPayload(ctx.desiredState,
                "ACTIVATION_FAILURE", "Self-update agent has failed to activate the new OS image.",
                "UPDATE_FAILURE", "Self-update agent has failed to activate the new OS image.",
                message, 0);
        case MqttMessage::Complete:
            return writeFeedbackWithPayload(ctx.desiredState,
                "COMPLETE", "Self-update completed.",
                ctx.desiredState.actionStatus, ctx.desiredState.actionMessage,
                message, 0);
        case MqttMessage::Incomplete:
            return writeFeedbackWithPayload(ctx.desiredState,
                "INCOMPLETE", "Self-update incomplete.",
                ctx.desiredState.actionStatus, ctx.desiredState.actionMessage,
                message, 0);
        }

        assert(false);
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
                            "id": "self-update:os-image",
                            "version": "{}",
                            "name": "Official Leda device image",
                            "type": "IMAGE"
                        }
                    ],
                    "hardwareNodes": [],
                    "associations": [
                        {
                            "sourceId": "self-update-agent",
                            "targetId": "self-update:os-image"
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
                            "id": "self-update:os-image",
                            "version": "{}",
                            "name": "Official Leda device image",
                            "type": "IMAGE"
                        }
                    ],
                    "hardwareNodes": [],
                    "associations": [
                        {
                            "sourceId": "self-update-agent",
                            "targetId": "self-update:os-image"
                        }
                    ]
                }
            }
        )");
        // clang-format on

        return fmt::format(tpl, activityId, epochTime(), SUA_BUILD_NUMBER, version);
    }

} // namespace sua
