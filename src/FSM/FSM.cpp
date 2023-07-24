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

#include "FSM/FSM.h"
#include "FSM/States/Uninitialized.h"
#include "Logger.h"

namespace sua {

    FSM::FSM(Context & context)
        : _context(context)
    { }

    void FSM::transitTo(const std::string& name)
    {
        assert(_factory != nullptr);

        if(_currentState) {
            Logger::trace("Leave state '{}'", _currentState->name());
            _currentState->onLeave(_context);
        }

        _currentState = _factory->createState(name);

        Logger::trace("Enter state '{}'", _currentState->name());
        _currentState->onEnter(_context);
    }

    std::string FSM::activeState() const
    {
        assert(_currentState != nullptr);

        return _currentState->name();
    }

    void FSM::setFactory(std::shared_ptr<StateFactory> factory)
    {
        _factory = factory;
    }

    void FSM::setTransitions(std::initializer_list<FSMTransition> table)
    {
        _transitions = table;
    }

    void FSM::handleEvent(const FotaEvent e)
    {
        Logger::trace("Received event '{}'", toString(e));

        FotaEvent output = FotaEvent::NotUsed;
        bool output_set  = false;

        for(const auto& t : _transitions) {
            if(t.from != activeState()) {
                continue;
            }

            if(t.when != e) {
                continue;
            }

            if(t.output == FotaEvent::NotUsed) {
                if(activeState() != t.to) {
                    transitTo(t.to);
                }
                break;
            }

            if(!output_set) {
                output     = _currentState->body(_context);
                output_set = true;
            }

            if(t.output == output) {
                if(activeState() != t.to) {
                    transitTo(t.to);
                }
                break;
            }
        }
    }

} // namespace sua
