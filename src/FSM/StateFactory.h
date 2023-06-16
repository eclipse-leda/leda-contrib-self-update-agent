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

#ifndef SDV_SUA_STATEFACTORY_H
#define SDV_SUA_STATEFACTORY_H

#include <functional>
#include <map>
#include <memory>

namespace sua {

    class State;

    class StateFactory {
    public:
        template <typename T>
        void addStateT(const std::string& name)
        {
            _states[name] = []() -> std::shared_ptr<State> { return std::make_shared<T>(); };
        }

        std::shared_ptr<State> createState(const std::string& name);

    private:
        std::map<std::string, std::function<std::shared_ptr<State>()>> _states;
    };

} // namespace sua

#endif
