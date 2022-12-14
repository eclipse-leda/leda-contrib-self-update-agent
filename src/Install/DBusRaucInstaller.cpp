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

#include "Install/DBusRaucInstaller.h"
#include "Logger.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <gio/gio.h>

namespace {
    const std::string VERSION_UNAVAILABLE = "bundle_version_not_available";
}

namespace sua {

    DBusRaucInstaller::DBusRaucInstaller()
    {
        Logger::trace("DBusRaucInstaller::DBusRaucInstaller()");
        setupDBusRaucConnection();
    }

    DBusRaucInstaller::~DBusRaucInstaller()
    {
        Logger::trace("DBusRaucInstaller::~DBusRaucInstaller()");
        if(nullptr != connection) {
            g_object_unref(connection);
        }
    }

    TechCode DBusRaucInstaller::installBundle(const std::string& input)
    {
        Logger::info("Installing rauc bundle {}", input);
        return installDBusRaucBundle(input);
    }

    int32_t DBusRaucInstaller::getInstallProgress()
    {
        return getDBusRaucInstallProgress();
    }

    std::string DBusRaucInstaller::getBundleVersion()
    {
        return getDBusRaucBundleVersion();
    }

    std::string DBusRaucInstaller::getBundleVersion(const std::string& input)
    {
        return getDBusRaucBundleVersion(input);
    }

    void DBusRaucInstaller::setupDBusRaucConnection()
    {
        GError* connectionError = nullptr;
        connection              = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &connectionError);

        if(nullptr != connection) {
            Logger::info("Valid connection to dbus");
        } else {
            Logger::error("Unable to connect to dbus, code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
        }
    }

    TechCode DBusRaucInstaller::installDBusRaucBundle(const std::string& bundleName)
    {
        GError*   connectionError = nullptr;
        GVariant* result          = g_dbus_connection_call_sync(connection,
                                                       "de.pengutronix.rauc",
                                                       "/",
                                                       "de.pengutronix.rauc.Installer",
                                                       "Install",
                                                       g_variant_new("(s)", bundleName.c_str()),
                                                       NULL,
                                                       G_DBUS_CALL_FLAGS_NONE,
                                                       -1,
                                                       NULL,
                                                       &connectionError);

        if(nullptr != result) {
            g_variant_unref(result);
        } else {
            Logger::error("Install call to Rauc via DBus failed, code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
            return TechCode::InstallationFailed;
        }

        return TechCode::OK;
    }

    int32_t DBusRaucInstaller::getDBusRaucInstallProgress() const
    {
        Logger::info("Install progress");
        GError*   connectionError = nullptr;
        GVariant* progressInfo    = g_dbus_connection_call_sync(
            connection,
            "de.pengutronix.rauc",
            "/",
            "org.freedesktop.DBus.Properties",
            "Get",
            g_variant_new("(ss)", "de.pengutronix.rauc.Installer", "Progress"),
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &connectionError);

        int32_t progressPercentage = 0;
        if(nullptr != progressInfo) {
            GVariant* internalVal;
            int32_t   nesting = 0;
            gchar*    message;
            g_variant_get(progressInfo, "(v)", &internalVal);
            g_variant_get(internalVal, "(isi)", &progressPercentage, &message, &nesting);
            Logger::info("Installing");
            Logger::info(" message             = {}", message);
            Logger::info(" progress percentage = {}", progressPercentage);
            Logger::info("nesting             = {}", nesting);
            g_variant_unref(progressInfo);
        } else {
            Logger::error("Connection to DBus lost? code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
        }
        return progressPercentage;
    }

    std::string DBusRaucInstaller::getDBusRaucBundleVersion() const
    {
        std::string bundleVersion   = VERSION_UNAVAILABLE;
        GError*     connectionError = nullptr;
        GVariant*   slotStatus      = g_dbus_connection_call_sync(connection,
                                                           "de.pengutronix.rauc",
                                                           "/",
                                                           "de.pengutronix.rauc.Installer",
                                                           "GetSlotStatus",
                                                           NULL,
                                                           NULL,
                                                           G_DBUS_CALL_FLAGS_NONE,
                                                           -1,
                                                           NULL,
                                                           &connectionError);

        if(nullptr != slotStatus) {
            GVariantIter* slotIter;
            gchar*        slotName;
            GVariant*     slotDict;
            uint32_t      bootedSlotCount = 0;
            g_variant_get(slotStatus, "(a(sa{sv}))", &slotIter);
            while(g_variant_iter_next(slotIter, "(s@a{sv})", &slotName, &slotDict)) {
                gchar*       slotState;
                gchar*       bundle;
                GVariantDict dict;
                g_variant_dict_init(&dict, slotDict);
                if(g_variant_dict_lookup(&dict, "state", "s", &slotState)) {
                    if("booted" == std::string(slotState)) {
                        bootedSlotCount++;
                        if(g_variant_dict_lookup(&dict, "bundle.version", "s", &bundle)) {
                            bundleVersion = bundle;
                        }
                    }
                }
            }

            if(bootedSlotCount > 1) {
                bundleVersion = "error_multiple_booted_slots_detected";
            }

            g_free(slotName);
            g_variant_unref(slotDict);
            g_variant_unref(slotStatus);
        }

        if(bundleVersion == VERSION_UNAVAILABLE) {
            std::ifstream f("/etc/os-release");
            if(f) {
                std::string keyValue;

                while(std::getline(f, keyValue)) {
                    std::stringstream ss(keyValue);

                    std::string key;
                    std::getline(ss, key, '=');

                    if(key == "VERSION_ID") {
                        std::string value;
                        std::getline(ss, value, '=');
                        value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
                        if(!value.empty()) {
                            bundleVersion = value;
                            break;
                        }
                    }
                }
            }
        }

        return bundleVersion;
    }

    std::string DBusRaucInstaller::getDBusRaucBundleVersion(const std::string& input) const
    {
        Logger::info("getDBusRaucBundleVersion, input={}", input);
        std::string bundleVersion   = "bundle_version_not_available";
        GError*     connectionError = nullptr;
        GVariant*   result          = g_dbus_connection_call_sync(connection,
                                                       "de.pengutronix.rauc",
                                                       "/",
                                                       "de.pengutronix.rauc.Installer",
                                                       "Info",
                                                       g_variant_new("(s)", input.c_str()),
                                                       NULL,
                                                       G_DBUS_CALL_FLAGS_NONE,
                                                       -1,
                                                       NULL,
                                                       &connectionError);

        if(nullptr != result) {
            Logger::info("Retrieved the version data, processing...");
            gchar* compatible;
            gchar* version;
            g_variant_get(result, "(ss)", &compatible, &version);
            bundleVersion = std::string(version);
            Logger::info("Version of downloaded bundle: {}", bundleVersion);
            g_free(compatible);
            g_free(version);
        } else {
            Logger::info("Retrieval of bundle version was not succesfull, error {}",
                         connectionError->message);
        }

        Logger::info("Retrieved version of the incoming bundle is: " + bundleVersion);
        return bundleVersion;
    }

} // namespace sua
