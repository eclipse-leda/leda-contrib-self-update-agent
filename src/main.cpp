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


#include "Install/DBusRaucInstaller.h"
#include "SoftwareUpdateAgent.h"

#include <iostream>
#include <memory>

#include <gio/gio.h>

#include "Logger.h"

int main(int argc, char* argv[])
{
    std::string hostPathToSelfupdateDir{"/data/selfupdates"};

    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            const std::string opt(argv[i]);
            if(("-h" == opt) || ("--help" == opt)) {
                std::cout << std::endl;
                std::cout << "Self Update Agent:" << std::endl;
                std::cout << "Usage: sdv-self-update-agent [OPTION]..." << std::endl;
                std::cout << "Perform an update of the system software." << std::endl;
                std::cout
                    << "The system needs to be restarted manually for the update to take effect."
                    << std::endl;
                std::cout << std::endl;
                std::cout << "  -h, --help  display this help and exit" << std::endl;
                std::cout << "  -p, --path  host path where downloaded update bundles will be "
                             "stored (default is "
                          << hostPathToSelfupdateDir << ")" << std::endl;
                std::cout << std::endl;
                return 0;
            }
            if(("-p" == opt) || ("--path" == opt)) {
                if((i + 1) < argc) {
                    hostPathToSelfupdateDir = argv[(i + 1)];
                }
            }
        }
    }

    sua::Logger::instance().init();
    sua::Logger::instance().setLogLevel(sua::Logger::Logger::All);
    sua::Logger::info("SoftwareUpdateAgent started");
    sua::Logger::info("Path to selfupdates directory = {}", hostPathToSelfupdateDir);

    std::unique_ptr<sua::SoftwareUpdateAgent> sua = std::make_unique<sua::SoftwareUpdateAgent>(
        std::make_shared<sua::DBusRaucInstaller>(), hostPathToSelfupdateDir);

    while(true) {
        sleep(1);
    }
    sua::Logger::trace("main.cpp finished");
    return 0;
}
