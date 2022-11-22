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

#ifndef SDV_SUA_IDLE_H
#define SDV_SUA_IDLE_H

#include "FSM/State.h"

namespace sua {
    class Idle : public State {
        using State::State;

    public:
        void      handleTemplate(FotaEvent event, const MessageState payload) override;
        FotaState getState() const override;

    protected:
        void onEntryTemplate() override;
        void adjustEntryPayloadTemplate() override;
    };
} // namespace sua

#endif
