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

#include "Download/Downloader.h"
#include "Install/DBusRaucInstaller.h"
#include "Mqtt/MqttMessagingProtocolJSON.h"
#include "Mqtt/MqttMessagingProtocolYAML.h"
#include "Mqtt/MqttConfiguration.h"
#include "Utils/BundleChecker.h"
#include "Utils/ServerAddressParser.h"
#include "SelfUpdateAgent.h"
#include "Logger.h"
#include "version.h"

#include <cstdlib>
#include <iostream>
#include <memory>

const char * help_string = R"(
Self Update Agent
Perform an update of the system software.
The system needs to be restarted manually for the update to take effect.

Usage: sdv-self-update-agent [OPTIONS]

Environment variables:
SUA_SERVER    sets and overrides MQTT server address to connect
              (default is 'tcp://mosquitto:1883')

Options:
-h, --help    display this help and exit
-v, --version display version (git hash and build number) used to build sua
-p, --path    path where downloaded update bundles will be stored (default is '/data/selfupdates')
-a, --api     use 'k8s' or 'bfb' format for mqtt communication (default is 'bfb')
-s, --server  MQTT broker server to connect (default is 'tcp://mosquitto:1883')
              (has precedence over SUA_SERVER environment variable)
)";

int main(int argc, char* argv[])
{
    std::string hostPathToSelfupdateDir{"/data/selfupdates"};
    std::string api    = "bfb";
    std::string server = "tcp://mosquitto:1883";

    const char * env_server = std::getenv("SUA_SERVER");
    if(env_server) {
        server = env_server;
    }

    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            const std::string opt(argv[i]);

            if(("-h" == opt) || ("--help" == opt)) {
                std::cout << help_string;
                return 0;
            }

            if(("-v" == opt) || ("--version" == opt)) {
                std::cout << "Build number    : " << SUA_BUILD_NUMBER << std::endl;
                std::cout << "Git commit hash : " << SUA_COMMIT_HASH  << std::endl;
                return 0;
            }

            if(("-p" == opt) || ("--path" == opt)) {
                if((i + 1) < argc) {
                    hostPathToSelfupdateDir = argv[(i + 1)];
                }
                continue;
            }

            if(("-a" == opt) || ("--api" == opt)) {
                if((i + 1) < argc) {
                    api = argv[i + 1];
                }
                continue;
            }

            if(("-s" == opt) || ("--server" == opt)) {
                if((i + 1) < argc) {
                    server = argv[i + 1];
                }
                continue;
            }
        }
    }

    bool failure_detected = false;

    std::cout << "Self Update Agent:" << std::endl;

    std::string transport = "tcp";
    std::string host      = "mosquitto";
    int         port      = 1883;

    try {
        sua::ServerAddressParser().parse(server, transport, host, port);
    } catch (const std::invalid_argument & e) {
        std::cout << fmt::format("Invalid port in address '{}'\n", server);
        failure_detected = true;
    } catch (const std::out_of_range & e) {
        std::cout << fmt::format("Invalid port in address '{}'\n", server);
        failure_detected = true;
    } catch (const std::runtime_error & e) {
        std::cout << e.what() << std::endl;
        failure_detected = true;
    }

    if(transport != "tcp") {
        std::cout << "Unsupported transport '" << transport << "', 'tcp' in one valid option." << std::endl;
        failure_detected = true;
    }

    if((api != "k8s") && (api != "bfb")) {
        std::cout << "Unsupported api '" << api << "', valid are 'k8s' and 'bfb' only."
                  << std::endl;
        failure_detected = true;
    }

    if(failure_detected) {
      return 0;
    }

    sua::Logger::instance().init();
    sua::Logger::instance().setLogLevel(sua::Logger::Logger::All);
    sua::Logger::info("SelfUpdateAgent started");
    sua::Logger::info("Path to selfupdates directory = {}", hostPathToSelfupdateDir);

    std::shared_ptr<sua::IMqttMessagingProtocol> protocol;
    if(api == "k8s") {
        protocol = std::make_shared<sua::MqttMessagingProtocolYAML>();
    } else {
        protocol = std::make_shared<sua::MqttMessagingProtocolJSON>();
    }

    // Depending on testing scenario, localhost or ip address of mosquitto container shall be used.
    sua::MqttConfiguration conf;
    conf.brokerHost = host;
    conf.brokerPort = port;

    sua::SelfUpdateAgent sua;
    sua::Context & ctx = sua.context();
    ctx.stateMachine      = std::make_shared<sua::FSM>(sua.context());
    ctx.installerAgent    = std::make_shared<sua::DBusRaucInstaller>();
    ctx.downloaderAgent   = std::make_shared<sua::Downloader>();
    ctx.messagingProtocol = protocol;
    ctx.updatesDirectory  = hostPathToSelfupdateDir;
    ctx.bundleChecker     = std::make_shared<sua::BundleChecker>();

    sua::Logger::info("SUA build number       : '{}'", SUA_BUILD_NUMBER );
    sua::Logger::info("SUA commit hash        : '{}'", SUA_COMMIT_HASH  );
    sua::Logger::info("Communication protocol : '{}'", api);
    sua::Logger::info("Updates directory      : '{}'", ctx.updatesDirectory);
    sua::Logger::info("MQTT broker address    : '{}://{}:{}'", transport, conf.brokerHost, conf.brokerPort);

    sua.init();
    sua.start(conf);

    while(true) {
        sleep(1);
    }

    return 0;
}
