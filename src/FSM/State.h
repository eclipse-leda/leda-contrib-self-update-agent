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

#ifndef SDV_SUA_STATE_H
#define SDV_SUA_STATE_H

#include "FotaEvent.h"

namespace sua {

    class Context;

    class State {
    public:
        State(const std::string& name = "");

        virtual ~State() = default;

        const std::string& name() const;

        virtual void onEnter(Context& ctx) {}
        virtual void onLeave(Context& ctx) {}

        virtual FotaEvent body(Context& ctx);

        void send(Context& ctx, const std::string& topic, const std::string& messageName, bool retained = false);
        void send(Context& ctx, const std::string& topic, const std::string& messageName, const std::string& message, bool retained = false);

    private:
        std::string _name;
    };

} // namespace sua

#endif
