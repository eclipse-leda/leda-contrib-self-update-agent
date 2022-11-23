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

#include "FSM/FSM.h"
#include "FSM/States/Uninitialized.h"
#include "Logger.h"

#include <memory>

namespace sua {
    FSM::FSM(const std::shared_ptr<IRaucInstaller> installerAgent,
             const std::string                     hostPathToUpdatesDir)
        : _installerAgent(installerAgent)
    {
        _selfupdatesDirPath = hostPathToUpdatesDir;
    }

    void FSM::start()
    {
        Logger::trace("FSM::start()");

        // set initial state
        auto                   thisPtr          = shared_from_this();
        std::shared_ptr<State> unitializedState = std::make_shared<Uninitialized>(thisPtr);

        // info: issue with current design is that the onEntry for the very first state is not executed
        // this can be easily changed, by making onEntry method public, however need to think about better design
        transitTo(unitializedState);
    }

    void FSM::handle(FotaEvent event, const MessageState payload)
    {
        if(_currentState) {
            _currentState->handle(event, payload);
        } else {
            throw std::runtime_error("FSM was not initialized! Not allowed to use it.");
        }
    }

    void FSM::transitTo(std::shared_ptr<State>& nextState)
    {
        _currentState = nextState;
    }
} // namespace sua
