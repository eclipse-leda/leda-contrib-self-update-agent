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

#ifndef SDV_SUA_IRAUCINSTALLER_H
#define SDV_SUA_IRAUCINSTALLER_H

#include "TechCodes.h"

#include <map>
#include <string>

namespace sua {

    // status["rootfs.0"]["version"] = "1.1"
    // status["rootfs.0"]["state"  ] = "booted"
    //
    // status["rootfs.1"]["version"] = "1.0"
    // status["rootfs.1"]["state"  ] = "inactive"
    using SlotStatus = std::map<std::string, std::map<std::string, std::string>>;

    class IRaucInstaller {
    public:
        virtual ~IRaucInstaller() = default;

        virtual TechCode    activateBooted()                            = 0;
        virtual TechCode    activateOther()                             = 0;
        virtual TechCode    installBundle(const std::string & input)    = 0;

        // Returns configurable poll interval in milliseconds
        // Value is implementation defined
        // Do not poll install status more often than this interval
        virtual int32_t     getProgressPollInterval() const             = 0;

        virtual int32_t     getInstallProgress()                        = 0;
        virtual SlotStatus  getSlotStatus()                             = 0;
        virtual std::string getBootedVersion()                          = 0;
        virtual std::string getBundleVersion(const std::string & input) = 0;
        virtual std::string getLastError()                              = 0;

        virtual bool installing() = 0;
        virtual bool succeeded()  = 0;
    };

} // namespace sua

#endif // SDV_SUA_IRAUCINSTALLER_H
