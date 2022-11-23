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

#include "FSM/State.h"
#include "Logger.h"
#include "Mqtt/MqttMessageSerializer.h"
#include "Patterns/Dispatcher.h"

namespace sua {
    std::string getStateName(const FotaState state)
    {
        switch(state) {
        case FotaState::Idle:
            return "idle";
        case FotaState::Downloading:
            return "downloading";
        case FotaState::Installing:
            return "installing";
        case FotaState::Installed:
            return "installed";
        case FotaState::Failed:
            return "failed";
        case FotaState::Uinitialized:
            return "uinitialized";
            ;
        default:
            return "unknown";
        }
    }

    std::string getEventName(const FotaEvent event)
    {
        switch(event) {
        case FotaEvent::ConnectivityEstablished:
            return "ConnectivityEstablished";
        case FotaEvent::ConnectivityLost:
            return "ConnectivityLost";
        case FotaEvent::FotaStart:
            return "FotaStart";
        case FotaEvent::DownloadProgress:
            return "DownloadProgress";
        case FotaEvent::DownloadReady:
            return "DownloadReady";
        case FotaEvent::DownloadError:
            return "DownloadError";
        case FotaEvent::InstallReady:
            return "InstallReady";
        case FotaEvent::InstallProgress:
            return "InstallProgress";
        case FotaEvent::InstallError:
            return "InstallError";
        default:
            return "Unknown";
        }
    }

    std::ostream& operator<<(std::ostream& os, FotaEvent event)
    {
        os << getEventName(event);
        return os;
    }

    std::ostream& operator<<(std::ostream& os, FotaState state)
    {
        os << getStateName(state);
        return os;
    }

    const std::string State::EVENT_PUBLISH = "State/Publish";
    const std::string State::EVENT_IDLE    = "State/Idle";

    void State::handle(FotaEvent event, const MessageState payload)
    {
        Logger::trace("State::handle({}, {})", static_cast<int>(event), toString(payload));
        handleTemplate(event, payload);
    }

    void State::handleBadEvent(FotaEvent event, const MessageState payload)
    {
        Logger::trace("State::handleBadEvent({}, {})", static_cast<int>(event), toString(payload));
    }

    MessageState State::getPayload() const
    {
        return _payload;
    }

    void State::setPayload(const MessageState payload)
    {
        _payload = payload;
    }

    void State::publishCurrentState() const
    {
        Logger::info("Publishing current state {} with payload = {}",
                     static_cast<int>(getState()),
                     toString(getPayload()));

        Dispatcher::instance().dispatch(EVENT_PUBLISH,
                                        MqttMessageSerializer().serialize(getPayload()));
    }

    void State::transitTo(std::shared_ptr<State>& nextState)
    {
        onExit();
        _context->transitTo(nextState);
        nextState->onEntry();
    }

    void State::onEntry()
    {
        Logger::info("Entering state {} with initial payload {}",
                     static_cast<int>(getState()),
                     toString(getPayload()));
        adjustEntryPayload();
        adjustEntryPayloadTemplate();
        publishCurrentState();
        onEntryTemplate();
    }

    void State::onExit()
    {
        Logger::info("Exiting state {} with payload {}",
                     static_cast<int>(getState()),
                     toString(getPayload()));
        onExitTemplate();
    }

    void State::onEntryTemplate() { }

    void State::onExitTemplate() { }

    void State::adjustEntryPayload()
    {
        _payload.stateName = getStateName(getState());
    }

    void State::adjustEntryPayloadTemplate() { }

} // namespace sua
