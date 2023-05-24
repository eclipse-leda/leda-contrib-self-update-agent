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

#include "FotaEvent.h"

#include <map>

namespace sua {

    std::string toString(const FotaEvent e)
    {
        // clang-format off
        static const std::map<FotaEvent, std::string> names = {
            { Waiting                   , "Waiting"                   },
            { ConnectivityEstablished   , "ConnectivityEstablished"   },
            { ConnectivityLost          , "ConnectivityLost"          },
            { Identify                  , "Identify"                  },
            { GetCurrentState           , "GetCurrentState"           },
            { BundleVersionOK           , "BundleVersionOK"           },
            { BundleVersionUnchanged    , "BundleVersionUnchanged"    },
            { BundleVersionInconsistent , "BundleVersionInconsistent" },
            { DownloadStart             , "DownloadStart"             },
            { DownloadFailed            , "DownloadFailed"            },
            { DownloadSucceeded         , "DownloadSucceeded"         },
            { InstallStart              , "InstallStart"              },
            { InstallCompleted          , "InstallCompleted"          },
            { InstallFailed             , "InstallFailed"             },
            { InstallFailedFallback     , "InstallFailedFallback"     },
            { Activate                  , "Activate"                  },
            { Cleanup                   , "Cleanup"                   },
            { Rollback                  , "Rollback"                  }
        };
        // clang-format on

        return names.at(e);
    }

} // namespace sua
