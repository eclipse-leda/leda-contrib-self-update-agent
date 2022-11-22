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

#include "FSM/FSM.h"
#include "Install/IRaucInstaller.h"
#include "PayloadMessages.h"

#include <memory>
#include <string>

namespace sua {
    enum class FotaEvent {
        ConnectivityEstablished,
        ConnectivityLost,
        FotaStart,
        DownloadProgress,
        DownloadReady,
        DownloadError,
        InstallReady,
        InstallProgress,
        InstallError
    };

    enum class FotaState { Uinitialized, Idle, Downloading, Installing, Failed, Installed };

    std::ostream& operator<<(std::ostream& os, FotaEvent event);
    std::ostream& operator<<(std::ostream& os, FotaState state);
    std::string   getStateName(const FotaState state);
    std::string   getEventName(const FotaEvent event);

    class State {
    public:
        static const std::string EVENT_PUBLISH;
        static const std::string EVENT_IDLE;

        State(std::shared_ptr<FSM>& context, const MessageState payload = MessageState())
            : _context(context)
            , _payload(payload)
        { }
        virtual ~State() = default;
        void handle(FotaEvent event, const MessageState payload = MessageState());

    protected:
        mutable MessageState _payload;
        std::shared_ptr<FSM> _context;

        virtual FotaState getState() const = 0;
        void              onEntry();
        void              onExit();
        void              transitTo(std::shared_ptr<State>& nextState);

        MessageState getPayload() const;
        void         setPayload(const MessageState payload);
        void         adjustEntryPayload();
        void         publishCurrentState() const;

        virtual void onEntryTemplate();
        virtual void handleTemplate(FotaEvent event, const MessageState payload) = 0;
        virtual void onExitTemplate();
        virtual void adjustEntryPayloadTemplate() = 0;
        void         handleBadEvent(FotaEvent event, const MessageState payload);
    };
} // namespace sua

#endif
