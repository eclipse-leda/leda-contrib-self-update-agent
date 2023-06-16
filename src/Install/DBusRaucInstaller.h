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

#ifndef SDV_SUA_DBUSRAUCINSTALLER_H
#define SDV_SUA_DBUSRAUCINSTALLER_H

#include "Install/IRaucInstaller.h"

#include <atomic>
#include <gio/gio.h>

namespace sua {

    class DBusRaucInstaller : public IRaucInstaller {
    public:
        DBusRaucInstaller();
        ~DBusRaucInstaller();

        TechCode    activateBooted() override;
        TechCode    activateOther() override;
        TechCode    installBundle(const std::string& input) override;
        int32_t     getProgressPollInterval() const override;
        int32_t     getInstallProgress() override;
        SlotStatus  getSlotStatus() override;
        std::string getBootedVersion() override;
        std::string getBundleVersion(const std::string& input) override;
        std::string getLastError() override;

        bool installing() override;
        bool succeeded() override;

        void setInstalling(bool value);
        void setSuccess(bool value);

    private:
        GDBusConnection* connection{nullptr};
        GMainLoop* loop{nullptr};

        guint            signalSubscriptionIdProperties;
        guint            signalSubscriptionIdCompleted;

        std::atomic_bool is_installing;
        std::atomic_bool is_succeeded;

        void        setupDBusRaucConnection();
        void        subscribeDBusSignals();
        void        unsubscribeDBusSignals();

        TechCode    callDBusRaucMark(const std::string& identifier, const std::string& state);
        TechCode    callDBusRaucInstallBundle(const std::string& bundleName);
        int32_t     getDBusRaucInstallProgress();
        std::string getDBusRaucProperty(const gchar* propertyKey) const;
        SlotStatus  getDBusRaucSlotStatus() const;
        std::string getDBusRaucBundleVersion(const std::string& input) const;
        std::string getOsVersionId(const std::string & version_key) const;
    };

} // namespace sua

#endif // SDV_SUA_DBUSRAUCINSTALLER_H
