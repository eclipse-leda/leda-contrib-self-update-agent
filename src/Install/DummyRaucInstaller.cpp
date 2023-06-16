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

#include "Install/DummyRaucInstaller.h"

namespace sua {

    DummyRaucInstaller::DummyRaucInstaller()
    { }

    DummyRaucInstaller::~DummyRaucInstaller()
    { }

    TechCode DummyRaucInstaller::installBundle(const std::string& input)
    {
        _progress = 0;
        return TechCode::OK;
    }

    int32_t DummyRaucInstaller::getProgressPollInterval() const
    {
        return 0;
    }

    int32_t DummyRaucInstaller::getInstallProgress()
    {
        return _progress;
    }

    SlotStatus DummyRaucInstaller::getSlotStatus()
    {
        SlotStatus s;

        s["rootfs.0"]["version"] = getBootedVersion();
        s["rootfs.0"]["state"  ] = "booted";

        return s;
    }

    std::string DummyRaucInstaller::getBootedVersion()
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

    bool DummyRaucInstaller::installing()
    {
        _progress = std::min(100, _progress + 20);
        return _progress != 100;
    }

    bool DummyRaucInstaller::succeeded()
    {
        return true;
    }

    TechCode DummyRaucInstaller::activateBooted()
    {
        return TechCode::OK;
    }

    TechCode DummyRaucInstaller::activateOther()
    {
        return TechCode::OK;
    }

} // namespace sua
