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

#include "StateFactory.h"
#include "State.h"

#include <stdexcept>

namespace sua {

    std::shared_ptr<State> StateFactory::createState(const std::string& name)
    {
        auto it = _states.find(name);

        if(it == _states.end()) {
            throw std::logic_error("Unknown state '" + name + "'");
        }

        return it->second();
    }

} // namespace sua
