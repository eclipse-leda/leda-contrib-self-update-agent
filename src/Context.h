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

#ifndef SDV_SUA_CONTEXT_H
#define SDV_SUA_CONTEXT_H

#include "FSM/FSM.h"
#include "Download/IDownloader.h"
#include "Install/IRaucInstaller.h"
#include "Mqtt/IMqttMessagingProtocol.h"
#include "Mqtt/IMqttProcessor.h"
#include "Utils/IBundleChecker.h"
#include "Defaults.h"

#include <memory>

namespace sua {

    struct DesiredState {
        std::string activityId;
        std::string bundleVersion;
        std::string bundleDownloadUrl;

        std::map<std::string, std::string> metadata;

        uint64_t downloadBytesTotal         = 0;
        uint64_t downloadBytesDownloaded    = 0;
        int      downloadProgressPercentage = 0;
        int      installProgressPercentage  = 0;

        std::string actionStatus               = "";
        std::string actionMessage              = "";
    };

    struct CurrentState {
        std::string version;
    };

    struct Context {
        std::shared_ptr<FSM>                    stateMachine;
        std::shared_ptr<IDownloader>            downloaderAgent;
        std::shared_ptr<IRaucInstaller>         installerAgent;
        std::shared_ptr<IMqttMessagingProtocol> messagingProtocol;
        std::shared_ptr<IMqttProcessor>         mqttProcessor;
        std::shared_ptr<IBundleChecker>         bundleChecker;
        std::string                             updatesDirectory = SUA_DEFAULT_TEMP_DIRECTORY;
        std::string                             tempFileName     = "/temp_file";
        std::string                             caDirectory      = SUA_DEFAULT_CA_DIRECTORY;
        std::string                             caFilepath       = SUA_DEFAULT_CA_FILEPATH;
        bool                                    downloadMode     = true;
        bool                                    fallbackMode     = false;

        DesiredState desiredState;
        CurrentState currentState;
    };

    struct Command {
        std::string activityId;
        FotaEvent   event;
    };

} // namespace sua

#endif
