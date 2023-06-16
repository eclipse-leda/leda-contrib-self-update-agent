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

#include "BundleChecker.h"
#include "Logger.h"

namespace sua {

    bool BundleChecker::isUpdateBundleVersionDifferent(const std::string & updateBundleVersion,
                                                       std::shared_ptr<IRaucInstaller> installer)
    {
        std::string slotVersion = installer->getBootedVersion();

        Logger::info("System version, installed: '{}'", slotVersion);
        Logger::info("Bundle version, from file: '{}'", updateBundleVersion);

        return slotVersion != updateBundleVersion;
    }

    bool BundleChecker::isBundleVersionConsistent(const std::string & declaredVersion,
                                                  std::shared_ptr<IRaucInstaller> installer,
                                                  const std::string & bundlePath)
    {
        std::string updateBundleVersion = installer->getBundleVersion(bundlePath);

        Logger::info("Bundle version, from spec: '{}'", declaredVersion);
        Logger::info("Bundle version, from file: '{}'", updateBundleVersion);

        return declaredVersion == updateBundleVersion;
    }

} // namespace sua
