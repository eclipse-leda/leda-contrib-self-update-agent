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

#include "Install/DummyRaucInstaller.h"

namespace sua {

    DummyRaucInstaller::DummyRaucInstaller()
    { }

    DummyRaucInstaller::~DummyRaucInstaller()
    { }

    TechCode DummyRaucInstaller::installBundle(const std::string& input)
    {
        return TechCode::OK;
    }

    int32_t DummyRaucInstaller::getInstallProgress()
    {
        static int progress = 0;
        progress += 1;
        progress = std::min(100, progress);
        return progress;
    }

    std::string DummyRaucInstaller::getBundleVersion()
    {
        return "dummy_version0";
    }

    std::string DummyRaucInstaller::getBundleVersion(const std::string& input)
    {
        return "dummy_version";
    }

    std::string DummyRaucInstaller::getLastError()
    {
        return "";
    }

} // namespace sua
