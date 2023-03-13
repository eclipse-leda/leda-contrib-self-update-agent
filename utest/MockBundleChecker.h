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

#ifndef SDV_SUA_MOCKBUNDLECHECKER_H
#define SDV_SUA_MOCKBUNDLECHECKER_H

#include "Utils/IBundleChecker.h"
#include "Install/IRaucInstaller.h"

#include "gmock/gmock.h"

class MockBundleChecker : public sua::IBundleChecker {
public:
    MOCK_METHOD(bool, isUpdateBundleVersionDifferent,
        (const std::string & updateBundleVersion, std::shared_ptr<sua::IRaucInstaller> installer), (override));
};

#endif

