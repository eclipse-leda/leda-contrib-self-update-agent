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

namespace sua {
    const std::string Installer::EVENT_INSTALLING = "Installer/Installing";
    const std::string Installer::EVENT_INSTALLED  = "Installed/Installed";
    const std::string Installer::EVENT_FAILED     = "Installed/Failed";

    Installer::Installer(std::shared_ptr<IRaucInstaller> installerAgent)
        : _installerAgent(installerAgent){};

    bool Installer::start(const std::string input)
    {
        Logger::trace("Installer::start({})", input);
        _isWorking = true;

        _installerAgent->installBundle(input);

        bool     installing         = true;
        uint32_t count              = 0;
        int32_t  progressPercentage = 0;
        while(installing) {
            progressPercentage = _installerAgent->getInstallProgress();
            sleep(2);
            count++;
            if(progressPercentage >= 100 || count >= 120) {
                installing = false;
            }
        }

        _isWorking = false;
        Dispatcher::instance().dispatch(EVENT_INSTALLED, "0");
        return _isWorking;
    }

} // namespace sua
