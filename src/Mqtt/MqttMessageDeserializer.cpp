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

#include "Mqtt/MqttMessageDeserializer.h"
#include "PayloadMessages.h"

#include "yaml/Yaml.hpp"

namespace sua {
    void MqttMessageDeserializer::deserialize(const std::string& payload, MessageStart& m) const
    {
        Yaml::Node root;
        Yaml::Parse(root, payload);

        m.apiVersion        = root["apiVersion"].As<std::string>();
        m.kind              = root["kind"].As<std::string>();
        m.metadataName      = root["metadata"]["name"].As<std::string>();
        m.bundleName        = root["spec"]["bundleName"].As<std::string>();
        m.bundleVersion     = root["spec"]["bundleVersion"].As<std::string>();
        m.bundleDownloadUrl = root["spec"]["bundleDownloadUrl"].As<std::string>();
        m.bundleTarget      = root["spec"]["bundleTarget"].As<std::string>();
    }

    void MqttMessageDeserializer::deserialize(const std::string& payload, MessageState& m) const
    {
        Yaml::Node root;
        Yaml::Parse(root, payload);

        m.apiVersion        = root["apiVersion"].As<std::string>();
        m.kind              = root["kind"].As<std::string>();
        m.metadataName      = root["metadata"]["name"].As<std::string>();
        m.bundleName        = root["spec"]["bundleName"].As<std::string>();
        m.bundleVersion     = root["spec"]["bundleVersion"].As<std::string>();
        m.bundleDownloadUrl = root["spec"]["bundleDownloadUrl"].As<std::string>();
        m.bundleTarget      = root["spec"]["bundleTarget"].As<std::string>();
        m.stateName         = root["state"]["name"].As<std::string>();
        m.stateProgress     = root["state"]["progress"].As<int>();
        m.stateTechCode     = root["state"]["techCode"].As<int>();
        m.stateMessage      = root["state"]["message"].As<std::string>();
    }
} // namespace sua
