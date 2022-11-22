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

#include "FSM/States/Idle.h"
#include "FSM/States/Downloading.h"
#include "FSM/States/Failed.h"
#include "FSM/States/Uninitialized.h"
#include "Patterns/Dispatcher.h"
#include "TechCodes.h"
#include "Utils/BundleChecker.h"

namespace sua {
    void Idle::onEntryTemplate()
    {
        Dispatcher::instance().dispatch(EVENT_IDLE, "");
    }

    void Idle::adjustEntryPayloadTemplate()
    {
        _payload.stateMessage  = "Entered Idle state";
        _payload.stateProgress = 0;
        _payload.stateTechCode = 0;
    }

    void Idle::handleTemplate(const FotaEvent event, const MessageState payload)
    {
        std::shared_ptr<State> nextState;

        switch(event) {
        case FotaEvent::FotaStart:
            if(BundleChecker().isUpdateBundleVersionDifferent(payload.bundleVersion,
                                                              _context->_installerAgent)) {
                nextState = std::make_shared<Downloading>(_context, payload);
            } else {
                MessageState msg  = payload;
                msg.stateProgress = 0;
                msg.stateTechCode = 4001;
                msg.stateMessage = "Update rejected, update bundle version same as current version";
                nextState        = std::make_shared<Failed>(_context, msg);
            }
            transitTo(nextState);
            break;
        case FotaEvent::ConnectivityLost:
            nextState = std::make_shared<Uninitialized>(_context, payload);
            transitTo(nextState);
            break;
        default:
            handleBadEvent(event, payload);
            break;
        }
    }

    FotaState Idle::getState() const
    {
        return FotaState::Idle;
    }
} // namespace sua