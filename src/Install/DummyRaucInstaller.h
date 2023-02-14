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

#ifndef SDV_SUA_DUMMYRAUCINSTALLER_H
#define SDV_SUA_DUMMYRAUCINSTALLER_H

#include "Install/IRaucInstaller.h"

namespace sua {

    // Only needed for initial setup & test, will be removed.
    class DummyRaucInstaller : public IRaucInstaller {
    public:
        DummyRaucInstaller();
        ~DummyRaucInstaller();

        TechCode    installBundle(const std::string& input) override;
        int32_t     getInstallProgress() override;
        std::string getBundleVersion() override;
        std::string getBundleVersion(const std::string& input) override;
        std::string getLastError() override;

        bool installing() override { return false; }
        bool succeeded() override { return false; }
    };

} // namespace sua

#endif // SDV_SUA_DUMMYRAUCINSTALLER_H
