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

#include "Mqtt/MqttMessageSerializer.h"
#include "PayloadMessages.h"

#include "yaml/Yaml.hpp"

namespace sua {
    std::string MqttMessageSerializer::serialize(const MessageState& m) const
    {
        Yaml::Node root;

        root["apiVersion"]       = m.apiVersion;
        root["kind"]             = m.kind;
        root["metadata"]["name"] = m.metadataName;

        root["spec"]["bundleName"]        = m.bundleName;
        root["spec"]["bundleVersion"]     = m.bundleVersion;
        root["spec"]["bundleDownloadUrl"] = m.bundleDownloadUrl;
        root["spec"]["bundleTarget"]      = m.bundleTarget;

        root["state"]["name"]     = m.stateName;
        root["state"]["progress"] = std::to_string(m.stateProgress);
        root["state"]["techCode"] = std::to_string(m.stateTechCode);
        root["state"]["message"]  = m.stateMessage;

        std::string serialized;
        Yaml::Serialize(root, serialized);
        return serialized;
    }

    std::string MqttMessageSerializer::serialize(const MessageCurrentState& m) const
    {
        Yaml::Node root;

        root["apiVersion"]            = m.apiVersion;
        root["kind"]                  = m.kind;
        root["metadata"]["name"]      = m.metadataName;
        root["spec"]["bundleVersion"] = m.bundleVersion;

        std::string serialized;
        Yaml::Serialize(root, serialized);
        return serialized;
    }
} // namespace sua
