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

#ifndef SDV_SUA_PAYLOADMESSAGES_H
#define SDV_SUA_PAYLOADMESSAGES_H

#include <ostream>
#include <sstream>
#include <string>

namespace sua {
    class MessageStart {
    public:
        std::string apiVersion;
        std::string kind;
        std::string metadataName;
        std::string bundleName;
        std::string bundleVersion;
        std::string bundleDownloadUrl;
        std::string bundleTarget;
    };

    class MessageState {
    public:
        std::string apiVersion;
        std::string kind;
        std::string metadataName;
        std::string bundleName;
        std::string bundleVersion;
        std::string bundleDownloadUrl;
        std::string bundleTarget;
        std::string stateName;
        int         stateProgress;
        int         stateTechCode;
        std::string stateMessage;
    };

    class MessageCurrentState {
    public:
        std::string apiVersion;
        std::string kind;
        std::string metadataName;
        std::string bundleVersion;
    };

    inline MessageState createFromStart(MessageStart start)
    {
        MessageState state;
        state.apiVersion        = start.apiVersion;
        state.kind              = start.kind;
        state.metadataName      = start.metadataName;
        state.bundleName        = start.bundleName;
        state.bundleVersion     = start.bundleVersion;
        state.bundleDownloadUrl = start.bundleDownloadUrl;
        state.bundleTarget      = start.bundleTarget;
        state.stateName         = "";
        state.stateProgress     = 0;
        state.stateTechCode     = 0;
        state.stateMessage      = "";
        return state;
    }

    inline std::string toString(MessageStart start)
    {
        std::stringstream ss;
        ss << "apiVersion: " << start.apiVersion << std::endl;
        ss << "kind: " << start.kind << std::endl;
        ss << "metadataName: " << start.metadataName << std::endl;
        ss << "bundleName: " << start.bundleName << std::endl;
        ss << "bundleVersion: " << start.bundleVersion << std::endl;
        ss << "bundleDownloadUrl: " << start.bundleDownloadUrl << std::endl;
        ss << "bundleTarget: " << start.bundleTarget << std::endl;
        return ss.str();
    }

    inline std::string toString(MessageState state)
    {
        std::stringstream ss;
        ss << "apiVersion: " << state.apiVersion << std::endl;
        ss << "kind: " << state.kind << std::endl;
        ss << "metadataName: " << state.metadataName << std::endl;
        ss << "bundleName: " << state.bundleName << std::endl;
        ss << "bundleVersion: " << state.bundleVersion << std::endl;
        ss << "bundleDownloadUrl: " << state.bundleDownloadUrl << std::endl;
        ss << "bundleTarget: " << state.bundleTarget << std::endl;
        ss << "stateName: " << state.stateName << std::endl;
        ss << "stateProgress: " << state.stateProgress << std::endl;
        ss << "stateTechCode: " << state.stateTechCode << std::endl;
        ss << "stateMessage: " << state.stateMessage << std::endl;
        return ss.str();
    }

    inline std::string toString(MessageCurrentState currentState)
    {
        std::stringstream ss;
        ss << "apiVersion: " << currentState.apiVersion << std::endl;
        ss << "kind: " << currentState.kind << std::endl;
        ss << "metadataName: " << currentState.metadataName << std::endl;
        ss << "bundleVersion: " << currentState.bundleVersion << std::endl;
        return ss.str();
    }

    inline std::ostream& operator<<(std::ostream& os, MessageState message)
    {
        os << toString(message);
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, MessageStart message)
    {
        os << toString(message);
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, MessageCurrentState message)
    {
        os << toString(message);
        return os;
    }
} // namespace sua

#endif
