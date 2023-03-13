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

#ifndef SDV_SUA_BUNDLECHECKER_H
#define SDV_SUA_BUNDLECHECKER_H

#include "IBundleChecker.h"

namespace sua {

    class BundleChecker : public IBundleChecker {
    public:
        bool isUpdateBundleVersionDifferent(const std::string &             updateBundleVersion,
                                            std::shared_ptr<IRaucInstaller> installer) override;

        bool isBundleVersionConsistent(const std::string &             declaredVersion,
                                       std::shared_ptr<IRaucInstaller> installer,
                                       const std::string &             bundlePath) override;
    };

} // namespace sua

#endif
