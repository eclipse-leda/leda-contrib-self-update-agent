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

#include "SoftwareUpdateAgent.h"
#include "Logger.h"
#include "Mqtt/MqttMessageDeserializer.h"

namespace sua {

    SoftwareUpdateAgent::SoftwareUpdateAgent(const std::shared_ptr<IRaucInstaller> installlerAgent,
                                             const std::string hostPathToUpdatesDir)
    {
        sua::MqttConfiguration conf;
        conf.brokerHost = "mosquitto";
        // "localhost"; // This is the IP of mosquitto on rpi yocto distro.
        // Depending on testing scenario, localhost or ip address of mosquitto container shall be used.
        conf.brokerPort = 1883;
        _mqttProcessor  = std::make_shared<MqttProcessor>(conf, this);

        _stateMachine = std::make_shared<FSM>(installlerAgent, hostPathToUpdatesDir);

        subscribe(State::EVENT_PUBLISH, [this](const std::string& payload) {
            MessageState m;
            MqttMessageDeserializer().deserialize(payload, m);
            _mqttProcessor->sendState(m);
        });

        subscribe(State::EVENT_IDLE, [this](const std::string& payload) {
            sua::MessageCurrentState msgCurrentState;
            msgCurrentState.apiVersion    = "sdv.eclipse.org/v1";
            msgCurrentState.kind          = "SelfUpdateBundle";
            msgCurrentState.metadataName  = "self-update-bundle-example";
            msgCurrentState.bundleVersion = _stateMachine->_installerAgent->getBundleVersion();
            _mqttProcessor->sendCurrentState(msgCurrentState);
        });

        subscribe(MqttProcessor::EVENT_DISCONNECTED, [this](const std::string&) {
            MessageState payload;
            payload.stateMessage  = "Connectivity lost";
            payload.stateTechCode = 1;
            _stateMachine->handle(FotaEvent::ConnectivityLost, payload);
        });

        subscribe(MqttProcessor::EVENT_CONNECTED, [this](const std::string&) {
            MessageState payload;
            payload.stateMessage = "Connectivity established";
            _stateMachine->handle(FotaEvent::ConnectivityEstablished);
        });

        start();
    }

    void SoftwareUpdateAgent::start()
    {
        Logger::trace("SoftwareUpdateAgent::start");
        _stateMachine->start();
        _mqttProcessor->start();
    }

    void SoftwareUpdateAgent::handle(const MessageStart& message)
    {
        Logger::trace("SoftwareUpdateAgent::handle(MessageStart)");
        MessageState messageState = createFromStart(message);
        _stateMachine->handle(FotaEvent::FotaStart, messageState);
    }

} // namespace sua
