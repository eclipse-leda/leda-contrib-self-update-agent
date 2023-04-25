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

#ifndef SDV_SUA_TECHCODES_H
#define SDV_SUA_TECHCODES_H

namespace sua {

    enum class TechCode {
        OK                 = 0,
        DownloadFailed     = 1001,
        InvalidBundle      = 2001,
        InstallationFailed = 3001,
        UpdateRejected     = 4001,
        UnknownError       = 5001
    };

} // namespace sua

#endif
