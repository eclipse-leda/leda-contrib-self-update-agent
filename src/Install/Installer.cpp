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

#include "Install/Installer.h"
#include "Logger.h"
#include "Patterns/Dispatcher.h"

#include <gio/gio.h>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace sua {

    const std::string Installer::EVENT_INSTALLING = "Installer/Installing";
        
    Installer::Installer(std::shared_ptr<IRaucInstaller> installerAgent)
        : _installerAgent(installerAgent)
    { }

    TechCode Installer::start(const std::string input)
    {
        Logger::trace("Installer::start({})", input);

        if(_installerAgent->installBundle(input) == TechCode::InstallationFailed) {
            return TechCode::InstallationFailed;
        }

        int32_t        progressPercentage          = 0;
        int32_t        progressNotificationLimiter = 0;
        uint32_t       waitingTime                 = 0;              // ms
        const uint32_t waitingTimeout              = 15 * 60 * 1000; // ms

        while(_installerAgent->installing()) {
            progressPercentage = _installerAgent->getInstallProgress();

            if(progressPercentage >= progressNotificationLimiter) {
                if(progressPercentage != 100) {
                    std::map<std::string, std::string> payload;
                    payload["percentage"] = std::to_string(progressPercentage);
                    sua::Dispatcher::instance().dispatch(EVENT_INSTALLING, payload);
                    progressNotificationLimiter = progressPercentage + 1;
                }
            }

            if(waitingTime >= waitingTimeout) {
                Logger::error("Waiting for completion more than {} secs => assuming failed installation", waitingTimeout);
                return TechCode::InstallationFailed;
            }

            if(progressPercentage < 100) {
                std::this_thread::sleep_for(std::chrono::milliseconds(_installerAgent->getProgressPollInterval()));
                waitingTime += _installerAgent->getProgressPollInterval();
            }
        }

        if(_installerAgent->succeeded()) {
            return TechCode::OK;
        }

        return TechCode::InstallationFailed;
    }

} // namespace sua
