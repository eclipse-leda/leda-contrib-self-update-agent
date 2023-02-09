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

    void on_completed(
                    GDBusConnection * connection,
                    const gchar * sender_name,
                    const gchar * object_path,
                    const gchar * interface_name,
                    const gchar * signal_name,
                    GVariant * parameters,
                    gpointer user_data)
    {
        Logger::trace("de.pengutronix.rauc.Installer::Completed callback");

        int32_t status = 0;
        g_variant_get(parameters, "(i)", &status);
        Logger::trace("RAUC install status = {}", status);

        DBusRaucInstaller * installer = (DBusRaucInstaller *)user_data;
        if(status == 0) {
            installer->setSuccess(true);
        } else {
            installer->setSuccess(false);
        }
        installer->setFinished();
    }

    DBusRaucInstaller::DBusRaucInstaller()
    {
        setupDBusRaucConnection();
    }

    DBusRaucInstaller::~DBusRaucInstaller()
    {
        if(nullptr != connection) {
            g_object_unref(connection);
        }

        if(nullptr != loop) {
            g_main_loop_unref(loop);
        }
    }

    TechCode DBusRaucInstaller::installBundle(const std::string& input)
    {
        Logger::trace("Installing rauc bundle {}", input);
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

        loop = g_main_loop_new(NULL, FALSE);

        auto id = g_dbus_connection_signal_subscribe(
                        connection,
                        "de.pengutronix.rauc",
                        "de.pengutronix.rauc.Installer",
                        "Completed",
                        "/",
                        NULL,
                        G_DBUS_SIGNAL_FLAGS_NONE,
                        on_completed,
                        this,
                        NULL
        );
        if(nullptr != connection) {
            Logger::trace("Valid connection to dbus");
        } else {
            Logger::error("Unable to connect to dbus, code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
        }
    }

    TechCode DBusRaucInstaller::installDBusRaucBundle(const std::string& bundleName)
    {
        is_installing = true;
        is_succeeded  = false;

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

            is_installing = false;
            is_succeeded  = false;

            return TechCode::InstallationFailed;
        }

        return TechCode::OK;
    }

    int32_t DBusRaucInstaller::getDBusRaucInstallProgress() const
    {
        Logger::trace("Install progress");
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
            Logger::trace("Installing");
            Logger::trace(" message             = {}", message);
            Logger::trace(" progress percentage = {}", progressPercentage);
            Logger::trace(" nesting             = {}", nesting);
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
        Logger::trace("getDBusRaucBundleVersion, input={}", input);
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
            Logger::trace("Retrieved the version data, processing...");
            gchar* compatible;
            gchar* version;
            g_variant_get(result, "(ss)", &compatible, &version);
            bundleVersion = std::string(version);
            Logger::trace("Version of downloaded bundle: {}", bundleVersion);
            g_free(compatible);
            g_free(version);
        } else {
            Logger::trace("Retrieval of bundle version was not succesfull, error {}",
                         connectionError->message);
        }

        Logger::trace("Retrieved version of the incoming bundle is: " + bundleVersion);
        return bundleVersion;
    }

    std::string DBusRaucInstaller::getLastError()
    {
        GError*   connectionError = nullptr;
        GVariant* lastErrorInfo   = g_dbus_connection_call_sync(
            connection,
            "de.pengutronix.rauc",
            "/",
            "org.freedesktop.DBus.Properties",
            "Get",
            g_variant_new("(ss)", "de.pengutronix.rauc.Installer", "LastError"),
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &connectionError);

        std::string lastError;
        if(nullptr != lastErrorInfo) {
            GVariant* internalVal;
            gchar*    message;
            g_variant_get(lastErrorInfo, "(v)", &internalVal);
            g_variant_get(internalVal, "s", &message);
            g_variant_unref(lastErrorInfo);
            lastError = message;
        } else {
            Logger::error("Connection to DBus lost? code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
        }
        return lastError;
    }

    bool DBusRaucInstaller::installing()
    {
        g_main_context_iteration(g_main_loop_get_context(loop), FALSE);
        return is_installing;
    }

    bool DBusRaucInstaller::succeeded()
    {
        return is_succeeded;
    }

    void DBusRaucInstaller::setSuccess(const bool value)
    {
        is_succeeded = value;
    }

    void DBusRaucInstaller::setFinished()
    {
        is_installing = false;
    }

} // namespace sua
