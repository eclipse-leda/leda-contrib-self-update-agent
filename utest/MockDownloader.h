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

#ifndef SDV_SUA_MOCKDOWNLOADER_H
#define SDV_SUA_MOCKDOWNLOADER_H

#include "Download/IDownloader.h"

#include "gmock/gmock.h"

class MockDownloader : public sua::IDownloader {
public:
    MOCK_METHOD(sua::TechCode, start, (const std::string & input), (override));
};

#endif