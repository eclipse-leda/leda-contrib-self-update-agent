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

#ifndef SDV_SUA_FSM_H
#define SDV_SUA_FSM_H

#include "Install/IRaucInstaller.h"
#include "PayloadMessages.h"

#include <memory>
#include <string>

namespace sua {
    enum class FotaEvent;
    enum class FotaState;
    class State;

    class FSM : public std::enable_shared_from_this<FSM> {
    public:
        FSM(const std::shared_ptr<IRaucInstaller> installerAgent,
            const std::string                     hostPathToUpdatesDir);

        void start();
        void handle(FotaEvent event, const MessageState payload = MessageState());
        void transitTo(std::shared_ptr<State>& nextState);

        std::shared_ptr<IRaucInstaller> _installerAgent;
        std::string                     _selfupdatesDirPath{"/data/selfupdates"};
        std::string                     _selfupdatesFilePath{_selfupdatesDirPath+"/temp_file"};

    protected:
        std::shared_ptr<State> _currentState;
    };
} // namespace sua

#endif
