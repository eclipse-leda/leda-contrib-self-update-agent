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

#include "Download/Downloader.h"
#include "Install/DBusRaucInstaller.h"
#include "Install/DummyRaucInstaller.h"
#include "Mqtt/MqttMessagingProtocolJSON.h"
#include "Mqtt/MqttConfiguration.h"
#include "Mqtt/MqttProcessor.h"
#include "Utils/BundleChecker.h"
#include "Utils/ServerAddressParser.h"
#include "Defaults.h"
#include "SelfUpdateAgent.h"
#include "Logger.h"
#include "version.h"

#include <cstdlib>
#include <iostream>
#include <memory>

const char * help_string_template = R"(
Self Update Agent
Perform an update of the system software.
The system needs to be restarted manually for the update to take effect.

Usage: sdv-self-update-agent [OPTIONS]

Environment variables:
SUA_SERVER         sets and overrides MQTT server address to connect
                   (default is '{}')

Options:
-h, --help         display this help and exit
-i, --installer    set install method 'download' to download update bundles and let Rauc install them,
                   'stream' to let Rauc install bundles directly from HTTP-server,
                   or 'dummy' for neither download nor installation
                   (default is '{}')
-p, --path         path where downloaded update bundles will be stored
                   (default is '{}')
-s, --server       MQTT broker server to connect, has precedence over SUA_SERVER environment variable
                   (default is '{}')
-c, --ca-directory a directory containing CA certificates to verify connection with bundle server
                   (default is '{}')
-f, --ca-filepath  file path to the CA certificate to verify connection with bundle server
                   if set it supersedes the CA directory in --ca-directory
                   (default is unset: '{}')
-v, --version      display version (Git hash and build number) used to build SUA and exit
)";

int main(int argc, char* argv[])
{
    std::string server                  = SUA_DEFAULT_MQTT_SERVER;
    std::string installer               = SUA_DEFAULT_MODE;
    std::string hostPathToSelfupdateDir = SUA_DEFAULT_TEMP_DIRECTORY;
    std::string caDirectory             = SUA_DEFAULT_CA_DIRECTORY;
    std::string caFilepath              = SUA_DEFAULT_CA_FILEPATH;
    std::string help_string             = fmt::format(help_string_template, server, installer, hostPathToSelfupdateDir, server, caDirectory, caFilepath);

    const char * env_server = std::getenv("SUA_SERVER");
    if(env_server) {
        server = env_server;
    }

    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            const std::string opt(argv[i]);
            std::string argValue;

            if(("-h" == opt) || ("--help" == opt)) {
                std::cout << help_string;
                return 0;
            }

            if(("-v" == opt) || ("--version" == opt)) {
                std::cout << "Build number    : " << SUA_BUILD_NUMBER << std::endl;
                std::cout << "Git commit hash : " << SUA_COMMIT_HASH  << std::endl;
                return 0;
            }

            if(i + 1 < argc && argv[i + 1][0] != '-') {
                i++;
                argValue = argv[i];
            }

            if(("-p" == opt) || ("--path" == opt)) {
                if (argValue.empty()) {
                    std::cout << "Missing path string for '" << opt << "'" << std::endl
                              << help_string << std::endl;
                    return 1;
                }
                hostPathToSelfupdateDir = argValue;
                continue;
            }

            if(("-s" == opt) || ("--server" == opt)) {
                if (argValue.empty()) {
                    std::cout << "Missing server string '" << opt << "'" << std::endl
                              << help_string << std::endl;
                    return 1;
                }
                server = argValue;
                continue;
            }

            if(("-i" == opt) || ("--installer" == opt)) {
                if (argValue.empty()) {
                    std::cout << "Missing install method for '" << opt << "'" << std::endl
                              << help_string << std::endl;
                    return 1;
                }
                installer = argValue;
                continue;
            }

            if(("-c" == opt) || ("--ca-path" == opt)) {
                if(argValue.empty()) {
                    std::cout << "Missing path to CA directory for '" << opt << "'" << std::endl
                              << help_string << std::endl;
                    return 1;
                }
                caDirectory = argValue;
                continue;
            }

            if(("-f" == opt) || ("--ca-file" == opt)) {
                if (argValue.empty()) {
                    std::cout << "Missing path to .crt for '" << opt << "'" << std::endl
                              << help_string << std::endl;
                    return 1;
                }
                caFilepath = argValue;
                continue;
            }

            std::cout << "Invalid argument '" << opt << "'" << std::endl
                      << help_string << std::endl;
            return 1;
        }
    }

    // check if arguments have valid values
    bool failure_detected = false;

    std::cout << "Self Update Agent:" << std::endl;

    std::string transport;
    std::string host;
    int         port;

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

    if((installer != "download") && (installer != "stream") && (installer != "dummy")) {
        std::cout << "Unsupported installer '" << installer
                  << "', valid are 'download', 'stream' and 'dummy' only." << std::endl;
        failure_detected = true;
    }

    if(failure_detected) {
        return 1;
    }

    sua::Logger::instance().init();
    sua::Logger::instance().setLogLevel(sua::Logger::Level::All);
    sua::Logger::info("SelfUpdateAgent started");

    std::shared_ptr<sua::IRaucInstaller> installerAgent;
    bool downloadMode = true;
    if(installer == "download") {
        installerAgent = std::make_shared<sua::DBusRaucInstaller>();
        downloadMode = true;
    }
    else if(installer == "stream") {
        installerAgent = std::make_shared<sua::DBusRaucInstaller>();
        downloadMode = false;
    } else {
        installerAgent = std::make_shared<sua::DummyRaucInstaller>();
        downloadMode = true;
    }

    // Depending on testing scenario, localhost or ip address of mosquitto container shall be used.
    sua::MqttConfiguration conf;
    conf.brokerHost = host;
    conf.brokerPort = port;

    sua::SelfUpdateAgent sua;
    sua::Context & ctx = sua.context();
    ctx.caDirectory       = caDirectory;
    ctx.caFilepath        = caFilepath;
    ctx.updatesDirectory  = hostPathToSelfupdateDir;
    ctx.stateMachine      = std::make_shared<sua::FSM>(sua.context());
    ctx.installerAgent    = installerAgent;
    ctx.downloadMode      = downloadMode;
    ctx.downloaderAgent   = std::make_shared<sua::Downloader>(ctx);
    ctx.messagingProtocol = std::make_shared<sua::MqttMessagingProtocolJSON>();
    ctx.bundleChecker     = std::make_shared<sua::BundleChecker>();
    ctx.mqttProcessor     = std::make_shared<sua::MqttProcessor>(ctx);

    sua::Logger::info("SUA build number       : '{}'", SUA_BUILD_NUMBER );
    sua::Logger::info("SUA commit hash        : '{}'", SUA_COMMIT_HASH  );
    sua::Logger::info("MQTT broker address    : '{}://{}:{}'", transport, conf.brokerHost, conf.brokerPort);
    sua::Logger::info("Install method         : '{}'", installer);
    sua::Logger::info("Updates directory      : '{}'", ctx.updatesDirectory);
    sua::Logger::info("CA directory           : '{}'", ctx.caDirectory);
    sua::Logger::info("CA certificate         : '{}'", ctx.caFilepath);

    sua.init();
    sua.start(conf);

    while(true) {
        sleep(1);
    }

    return 0;
}
