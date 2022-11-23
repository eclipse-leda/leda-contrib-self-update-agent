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

#include "FSM/States/Failed.h"
#include "FSM/States/Idle.h"
#include "FSM/States/Uninitialized.h"

namespace sua {
    void Failed::onEntryTemplate()
    {
        // TODO, depending on the payload, perform proper cleanup
        // for example deleting broken file
        std::shared_ptr<State> nextState = std::make_shared<Idle>(_context);
        transitTo(nextState);
    }

    void Failed::adjustEntryPayloadTemplate()
    {
        // Empty, use gived payload
    }

    void Failed::handleTemplate(const FotaEvent event, const MessageState payload)
    {
        std::shared_ptr<State> nextState;

        switch(event) {
        case FotaEvent::ConnectivityLost:
            nextState = std::make_shared<Uninitialized>(_context, payload);
            transitTo(nextState);
            break;
        default:
            handleBadEvent(event, payload);
            break;
        }
    }

    FotaState Failed::getState() const
    {
        return FotaState::Failed;
    }
} // namespace sua
