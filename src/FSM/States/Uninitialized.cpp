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

#include "FSM/States/Uninitialized.h"
#include "FSM/States/Idle.h"

namespace sua {

    void Uninitialized::adjustEntryPayloadTemplate()
    {
        // use passed values
    }

    void Uninitialized::handleTemplate(FotaEvent event, const MessageState payload)
    {
        std::shared_ptr<State> nextState;

        switch(event) {
        case FotaEvent::ConnectivityEstablished:
            nextState = std::make_shared<Idle>(_context, payload);
            transitTo(nextState);
            break;

        default:
            handleBadEvent(event, payload);
            break;
        }
    }

    FotaState Uninitialized::getState() const
    {
        return FotaState::Uinitialized;
    }
} // namespace sua
