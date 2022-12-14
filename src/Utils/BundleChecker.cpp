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

#include "BundleChecker.h"

namespace sua {

    bool
    BundleChecker::isUpdateBundleVersionDifferent(const std::string & updateBundleVer,
                                                  std::shared_ptr<IRaucInstaller> installer)
    {
        return updateBundleVer != installer->getBundleVersion();
    }

    bool BundleChecker::isBundleVersionConsistent(const std::string & declaredVersion,
                                                  std::shared_ptr<IRaucInstaller> installer,
                                                  const std::string &             bundlePath)
    {
        return declaredVersion == installer->getBundleVersion(bundlePath);
    }

} // namespace sua
