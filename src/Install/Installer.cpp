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

        if(_installerAgent->installBundle(input) != TechCode::OK) {
            return TechCode::InstallationFailed;
        }

        bool     installing                  = true;
        uint32_t count                       = 0;
        int32_t  progressPercentage          = 0;
        int32_t  progressNotificationLimiter = 0;

        while(installing) {
            progressPercentage = _installerAgent->getInstallProgress();
            std::this_thread::sleep_for(2000ms);
            count++;
            if(progressPercentage >= 100 || count >= 120) {
                installing = false;
            }

            if(progressPercentage >= progressNotificationLimiter) {
                if(progressPercentage != 100) {
                    std::map<std::string, std::string> payload;
                    payload["percentage"] = std::to_string(progressPercentage);
                    sua::Dispatcher::instance().dispatch(EVENT_INSTALLING, payload);
                }
                progressNotificationLimiter += 10;
            }
        }

        return TechCode::OK;
    }

} // namespace sua
