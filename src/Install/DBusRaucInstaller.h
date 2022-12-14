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

#ifndef SDV_SUA_DBUSRAUCINSTALLER_H
#define SDV_SUA_DBUSRAUCINSTALLER_H

#include "Install/IRaucInstaller.h"

#include <gio/gio.h>

namespace sua {

    class DBusRaucInstaller : public IRaucInstaller {
    public:
        DBusRaucInstaller();
        ~DBusRaucInstaller();

        TechCode    installBundle(const std::string& input) override;
        int32_t     getInstallProgress() override;
        std::string getBundleVersion() override;
        std::string getBundleVersion(const std::string& input) override;

    private:
        GDBusConnection* connection{nullptr};

        void        setupDBusRaucConnection();
        TechCode    installDBusRaucBundle(const std::string& bundleName);
        int32_t     getDBusRaucInstallProgress() const;
        std::string getDBusRaucBundleVersion() const;
        std::string getDBusRaucBundleVersion(const std::string& input) const;
    };

} // namespace sua

#endif // SDV_SUA_DBUSRAUCINSTALLER_H
