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
#include "Logger.h"

namespace sua {

    static bool areVersionsTheSame(const std::string ver1, const std::string ver2){
        if(ver1 == ver2) {
            Logger::info("Versions are matching, val1={}, val2={}", ver1, ver2);
            return true;

        } else {
            Logger::info("Versions are not matching, val1={}, val2={}", ver1, ver2);
            return false;
        }
    }

    bool BundleChecker::isUpdateBundleVersionDifferent(const std::string updateBundleVer,
                                                  const std::shared_ptr<IRaucInstaller> installer)
    {
        bool versionsMatching = areVersionsTheSame(updateBundleVer, installer->getBundleVersion());

        if(versionsMatching) {
            Logger::info("Incoming update version {} and current version are the same, do nothing",
                         updateBundleVer);
            return false;
        } else {
            Logger::info("Incoming update version {} and current version are not the same, will perform the update",
                         updateBundleVer);
            return true;
        }
    }

    bool BundleChecker::isBundleVersionConsistent(const std::string declaredVersion,
                                                  const std::shared_ptr<IRaucInstaller> installer,
                                                  const std::string bundlePath)
    {
        bool versionsMatching = areVersionsTheSame(declaredVersion, installer->getBundleVersion(bundlePath));

        if(versionsMatching) {
            Logger::info("Incoming declared update version {} and downloaded bundle version are the same, request is valid",
                         declaredVersion);
            return true;
        } else {
            Logger::info("Incoming declared update version {} and downloaded bundle version are not the same, request is not valid",
                         declaredVersion);
            return false;
        }
    }

    

} // namespace sua
