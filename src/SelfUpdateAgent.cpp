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

#include "SelfUpdateAgent.h"
#include "FSM/States/Connected.h"
#include "FSM/States/Downloading.h"
#include "FSM/States/Failed.h"
#include "FSM/States/Idle.h"
#include "FSM/States/Installed.h"
#include "FSM/States/Installing.h"
#include "FSM/States/SendCurrentState.h"
#include "FSM/States/Uninitialized.h"
#include "FotaEvent.h"
#include "Logger.h"
#include "Mqtt/MqttProcessor.h"

namespace sua {

    SelfUpdateAgent::SelfUpdateAgent()
    { }

    void SelfUpdateAgent::init()
    {
        // clang-format off
        auto factory = std::make_shared<StateFactory>();
        factory->addStateT<Connected       >("Connected"       );
        factory->addStateT<Downloading     >("Downloading"     );
        factory->addStateT<Failed          >("Failed"          );
        factory->addStateT<Idle            >("Idle"            );
        factory->addStateT<Installed       >("Installed"       );
        factory->addStateT<Installing      >("Installing"      );
        factory->addStateT<SendCurrentState>("SendCurrentState");
        factory->addStateT<Uninitialized   >("Uninitialized"   );
        _context.stateMachine->setFactory(factory);
        // clang-format-on

        // clang-format off
        _context.stateMachine->setTransitions({
            // from "Uninitialized"
            { FotaEvent::ConnectivityEstablished, "Uninitialized"   , "Connected"       },
            // from "Connected"
            { FotaEvent::ConnectivityLost       , "Connected"       , "Uninitialized"   },
            { FotaEvent::Start                  , "Connected"       , "Failed"           , FotaEvent::BundleVersionUnchanged },
            { FotaEvent::Start                  , "Connected"       , "Downloading"      , FotaEvent::BundleVersionOK        },
            { FotaEvent::GetCurrentState        , "Connected"       , "SendCurrentState"},
            // from "Failed"
            { FotaEvent::ConnectivityLost       , "Failed"          , "Uninitialized"   },
            { FotaEvent::Waiting                , "Failed"          , "Idle"            },
            // from "Downloading"
            { FotaEvent::ConnectivityLost       , "Downloading"     , "Uninitialized"   },
            { FotaEvent::DownloadStart          , "Downloading"     , "Installing"       , FotaEvent::BundleVersionOK           },
            { FotaEvent::DownloadStart          , "Downloading"     , "Failed"           , FotaEvent::BundleVersionInconsistent },
            { FotaEvent::DownloadStart          , "Downloading"     , "Failed"           , FotaEvent::DownloadFailed            },
            // from "Installing"
            { FotaEvent::ConnectivityLost       , "Installing"      , "Uninitialized"   },
            { FotaEvent::InstallStart           , "Installing"      , "Installed"        , FotaEvent::InstallCompleted          },
            { FotaEvent::InstallStart           , "Installing"      , "Failed"           , FotaEvent::InstallFailed             },
            // from "Installed"
            { FotaEvent::ConnectivityLost       , "Installed"       , "Uninitialized"   },
            { FotaEvent::Waiting                , "Installed"       , "Idle"            },
            // from "Idle"
            { FotaEvent::ConnectivityLost       , "Idle"            , "Uninitialized"   },
            { FotaEvent::Start                  , "Idle"            , "Failed"           , FotaEvent::BundleVersionUnchanged },
            { FotaEvent::Start                  , "Idle"            , "Downloading"      , FotaEvent::BundleVersionOK        },
            // from "SendCurrentState"
            { FotaEvent::ConnectivityLost       , "SendCurrentState", "Uninitialized"   },
            { FotaEvent::Waiting                , "SendCurrentState", "Idle"            },
        });
        _context.stateMachine->transitTo("Uninitialized");
        // clang-format on
    }

    void SelfUpdateAgent::start(const MqttConfiguration & config)
    {
        assert(_context.stateMachine      != nullptr);
        assert(_context.downloaderAgent   != nullptr);
        assert(_context.installerAgent    != nullptr);
        assert(_context.messagingProtocol != nullptr);
        assert(_context.bundleChecker     != nullptr);

        _context.mqttProcessor = std::make_shared<MqttProcessor>(config, _context);
        _context.mqttProcessor->start();
    }

    Context& SelfUpdateAgent::context()
    {
        return _context;
    }

} // namespace sua
