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

#include "FSM/States/Installing.h"
#include "FSM/States/Failed.h"
#include "FSM/States/Installed.h"
#include "FSM/States/Uninitialized.h"
#include "Install/Installer.h"
#include "Logger.h"

namespace sua {
    void cleanUpInstallation()
    {
        // TODO, add logic to clean up the Installation
        Logger::info("Cleaning up installation");
    }

    Installing::Installing(std::shared_ptr<FSM>& context, const MessageState payload)
        : State(context, payload)
    {
        subscribe(Installer::EVENT_INSTALLED, [this](const std::string& payload) {
            Logger::info("New state for Installing: Installed, payload = {}", payload);
            _payload.stateTechCode = std::stoi(payload);
            _payload.stateProgress = 0;
            _payload.stateMessage  = "Installing completed";
            handle(FotaEvent::InstallReady, _payload);
        });

        subscribe(Installer::EVENT_INSTALLING, [this](const std::string& payload) {
            Logger::info("New state for Installing: Installing, payload = {}", payload);
            _payload.stateProgress = std::stoi(payload);
            _payload.stateTechCode = 0;
            _payload.stateMessage  = "Installing, progress = " + payload;
            handle(FotaEvent::InstallProgress, _payload);
        });

        subscribe(Installer::EVENT_FAILED, [this](const std::string& payload) {
            Logger::info("New state for Installing: Failed, payload = {}", payload);
            _payload.stateTechCode = std::stoi(payload);
            _payload.stateProgress = 0;
            _payload.stateMessage  = "Installing failed with error code = " + payload;
            handle(FotaEvent::InstallError, _payload);
        });
    }

    void Installing::onEntryTemplate()
    {
        _installer = std::make_unique<Installer>(_context->_installerAgent);
        _installer->start(_context->_selfupdatesFilePath);
    }

    void Installing::adjustEntryPayloadTemplate()
    {
        _payload.stateMessage  = "Entered Installing state";
        _payload.stateProgress = 0;
        _payload.stateTechCode = 0;
    }

    void Installing::handleTemplate(const FotaEvent event, const MessageState payload)
    {
        std::shared_ptr<State> nextState;

        switch(event) {
        case FotaEvent::InstallReady:
            Logger::trace("INSTALLATION DONE");
            nextState = std::make_shared<Installed>(_context, payload);
            transitTo(nextState);
            break;
        case FotaEvent::InstallProgress:
            Logger::trace("INSTALLATION Progressed");
            setPayload(payload);
            publishCurrentState();
            break;
        case FotaEvent::InstallError:
            Logger::trace("INSTALLATION FAILED");
            cleanUpInstallation();
            nextState = std::make_shared<Failed>(_context, payload);
            transitTo(nextState);
            break;
        case FotaEvent::ConnectivityLost:
            Logger::trace("CONNECTION LOST during installation");
            cleanUpInstallation();
            // TODO, figure out what is the best strategy to handle this, we can continue installation...
            nextState = std::make_shared<Uninitialized>(_context);
            transitTo(nextState);
            break;

        default:
            handleBadEvent(event, payload);
            break;
        }
    }

    FotaState Installing::getState() const
    {
        return FotaState::Installing;
    }
} // namespace sua
