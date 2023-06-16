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

#ifndef SDV_SUA_FSM_H
#define SDV_SUA_FSM_H

#include "StateFactory.h"
#include "FotaEvent.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace sua {

    struct FSMTransition {
        FotaEvent   when;
        std::string from;
        std::string to;
        FotaEvent   output = FotaEvent::NotUsed;
    };

    class FSM {
    public:
        FSM(class Context & context);

        void handleEvent(FotaEvent e);

        virtual void transitTo(const std::string& name);
        std::string activeState() const;

        void setFactory(std::shared_ptr<StateFactory> factory);
        void setTransitions(std::initializer_list<FSMTransition> table);

    private:
        Context & _context;

        std::shared_ptr<State>        _currentState;
        std::shared_ptr<StateFactory> _factory;
        std::vector<FSMTransition>    _transitions;
    };
} // namespace sua

#endif
