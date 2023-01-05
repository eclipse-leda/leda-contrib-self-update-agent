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

#include "MqttMessagingProtocolYAML.h"
#include "Context.h"

#include "yaml/Yaml.hpp"
#include "spdlog/fmt/fmt.h"

#include <map>

namespace {

    Yaml::Node base(const sua::DesiredState & desiredState, const std::map<std::string, std::string> &extraFields = {})
    {
        Yaml::Node root;

        auto safeSet = [&root, &extraFields] (const std::string&key, const std::string& yamlKey) {
            if(extraFields.find(key) != extraFields.end()) {
                root["state"][yamlKey] = extraFields.at(key);
            }
        };

        // clang-format off
        root["apiVersion"]                = desiredState.metadata.at("apiVersion");
        root["kind"]                      = desiredState.metadata.at("kind");
        root["metadata"]["name"]          = desiredState.metadata.at("name");
        root["spec"]["bundleName"       ] = desiredState.metadata.at("bundleName");
        root["spec"]["bundleVersion"    ] = desiredState.bundleVersion;
        root["spec"]["bundleDownloadUrl"] = desiredState.bundleDownloadUrl;
        root["spec"]["bundleTarget"     ] = desiredState.metadata.at("bundleTarget");

        safeSet("state/name"    , "name"    );
        safeSet("state/progress", "progress");
        safeSet("state/techCode", "techCode");
        safeSet("state/message" , "message" );
        // clang-format on

        return root;
    }

    std::string serialized(const Yaml::Node& root)
    {
        std::string serialized;
        Yaml::Serialize(root, serialized);
        return serialized;
    }

}

namespace sua {

    DesiredState MqttMessagingProtocolYAML::readDesiredState(const std::string & input)
    {
        Yaml::Node root;
        Yaml::Parse(root, input);

        // clang-format off
        DesiredState s;
        s.bundleVersion            = root["spec"]["bundleVersion"    ].As<std::string>();
        s.bundleDownloadUrl        = root["spec"]["bundleDownloadUrl"].As<std::string>();
        s.metadata["apiVersion"]   = root["apiVersion"].As<std::string>();
        s.metadata["kind"]         = root["kind"].As<std::string>();
        s.metadata["name"]         = root["metadata"]["name"].As<std::string>();
        s.metadata["bundleName"]   = root["spec"]["bundleName"].As<std::string>();
        s.metadata["bundleTarget"] = root["spec"]["bundleTarget"].As<std::string>();
        return s;
        // clang-format on
    }

    DesiredState MqttMessagingProtocolYAML::readCurrentStateRequest(const std::string & input)
    {
      // ignored, no activity id needed, simply pass empty result and trigger response
      return {};
    }

    std::string MqttMessagingProtocolYAML::createMessage(const class Context& ctx, const std::string& name)
    {
        Yaml::Node root;
        root["apiVersion"]       = "sdv.eclipse.org/v1";
        root["kind"]             = "SelfUpdateBundle";
        root["metadata"]["name"] = "self-update-bundle-example";

        if(name == "systemVersion") {
            root["spec"]["bundleVersion"] = ctx.currentState.version;
            return serialized(root);
        }

        if(name == "identifying") {
            return "";
        }

        if(name == "identified") {
            return "";
        }

        if(name == "identifiedAndSkipped") {
            return "";
        }

        if(name == "completedAndSkipped") {
            return "";
        }

        if(name == "downloading") {
            const double mbytes = static_cast<double>(ctx.desiredState.downloadBytesTotal) / 1024.0 / 1024.0;

            std::map<std::string, std::string> state;
            state["state/name"    ] = "downloading";
            state["state/progress"] = std::to_string(ctx.desiredState.downloadProgressPercentage);
            state["state/message" ] = fmt::format("Downloading {:.{}f} MiB...", mbytes, 1);
            return serialized(base(ctx.desiredState, state));
        }

        if(name == "downloaded") {
            const double mbytes = static_cast<double>(ctx.desiredState.downloadBytesTotal) / 1024.0 / 1024.0;

            std::map<std::string, std::string> state;
            state["state/name"    ] = "downloading";
            state["state/progress"] = "100";
            state["state/message" ] = fmt::format("Downloaded {:.{}f} MiB...", mbytes, 1);
            return serialized(base(ctx.desiredState, state));
        }

        if(name == "installing") {
            std::map<std::string, std::string> state;
            state["state/name"    ] = "installing";
            state["state/progress"] = std::to_string(ctx.desiredState.installProgressPercentage);
            state["state/message" ] = fmt::format("RAUC install...");
            return serialized(base(ctx.desiredState, state));
        }

        if(name == "installed") {
            std::map<std::string, std::string> state;
            state["state/name"    ] = "installed";
            state["state/progress"] = "100";
            state["state/message" ] = fmt::format("RAUC install completed...");
            return serialized(base(ctx.desiredState, state));
        }

        if(name == "currentState") {
            std::map<std::string, std::string> state;
            state["state/name"    ] = "idle";
            state["state/message" ] = "Idle";
            return serialized(base(ctx.desiredState, state));
        }

        if(name == "downloadFailed") {
            std::map<std::string, std::string> state;
            state["state/name"    ] = "failed";
            state["state/message" ] = "Download failed";
            state["state/techCode"] = "1001";
            return serialized(base(ctx.desiredState, state));
        }

        if(name == "invalidBundle") {
            std::map<std::string, std::string> state;
            state["state/name"    ] = "failed";
            state["state/message" ] = "Invalid bundle";
            state["state/techCode"] = "2001";
            return serialized(base(ctx.desiredState, state));
        }

        if(name == "installFailed") {
            std::map<std::string, std::string> state;
            state["state/name"    ] = "failed";
            state["state/message" ] = "Install failed";
            state["state/techCode"] = "3001";
            return serialized(base(ctx.desiredState, state));
        }

        if(name == "updateRejected") {
            std::map<std::string, std::string> state;
            state["state/name"    ] = "failed";
            state["state/message" ] = "Update rejected";
            state["state/techCode"] = "4001";
            return serialized(base(ctx.desiredState, state));
        }

        throw std::logic_error(fmt::format("Unknown message type '{}'", name));
    }

} // namespace su
