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
#include "Context.h"

namespace sua {

    State::State(const std::string& name)
        : _name(name)
    { }

    const std::string& State::name() const
    {
        return _name;
    }

    FotaEvent State::body(Context& /*ctx*/)
    {
        return FotaEvent::NotUsed;
    }

    void State::send(Context& ctx, const std::string& topic, const std::string& messageName, bool retained)
    {
        ctx.mqttProcessor->send(topic, ctx.messagingProtocol->createMessage(ctx, messageName), retained);
    }

    void State::send(Context& ctx, const std::string& topic, const std::string& messageName, const std::string& message, bool retained)
    {
        ctx.mqttProcessor->send(topic, ctx.messagingProtocol->createMessage(ctx, messageName, message), retained);
    }

} // namespace sua
