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

#include "MqttMessage.h"

#include <map>

namespace sua {

    std::ostream& operator<<(std::ostream& os, MqttMessage m)
    {
        // clang-format off
        static const std::map<sua::MqttMessage, std::string> names = {
            { sua::MqttMessage::SystemVersion        , "SystemVersion"        },
            { sua::MqttMessage::CurrentState         , "CurrentState"         },
            { sua::MqttMessage::Identifying          , "Identifying"          },
            { sua::MqttMessage::Identified           , "Identified"           },
            { sua::MqttMessage::IdentificationFailed , "IdentificationFailed" },
            { sua::MqttMessage::Skipped              , "Skipped"              },
            { sua::MqttMessage::Rejected             , "Rejected"             },
            { sua::MqttMessage::Downloading          , "Downloading"          },
            { sua::MqttMessage::Downloaded           , "Downloaded"           },
            { sua::MqttMessage::DownloadFailed       , "DownloadFailed"       },
            { sua::MqttMessage::VersionChecking      , "VersionChecking"      },
            { sua::MqttMessage::Installing           , "Installing"           },
            { sua::MqttMessage::Installed            , "Installed"            },
            { sua::MqttMessage::InstallFailed        , "InstallFailed"        },
            { sua::MqttMessage::InstallFailedFallback, "InstallFailedFallback"},
            { sua::MqttMessage::Cleaned              , "Cleaned"              },
            { sua::MqttMessage::Activating           , "Activating"           },
            { sua::MqttMessage::Activated            , "Activated"            },
            { sua::MqttMessage::ActivationFailed     , "ActivationFailed"     },
            { sua::MqttMessage::Complete             , "Complete"             },
            { sua::MqttMessage::Incomplete           , "Incomplete"           },

        };
        // clang-format on

        return os << names.at(m);
    }

} // namespace sua
