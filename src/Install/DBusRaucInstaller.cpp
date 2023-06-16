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

#include "Install/DBusRaucInstaller.h"
#include "Logger.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <gio/gio.h>

namespace {

    const std::string MULTIPLE_BOOTED_SLOTS = "error_multiple_booted_slots_detected";
    const std::string VERSION_UNAVAILABLE   = "bundle_version_not_available";
    const std::string STATE_UNAVAILABLE     = "slot_state_not_available";

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
        int32_t status = 0;
        g_variant_get(parameters, "(i)", &status);
        Logger::trace("Rauc signals Completed: install status = {}", status);

        DBusRaucInstaller * installer = (DBusRaucInstaller *)user_data;
        if(status == 0) {
            installer->setSuccess(true);
        } else {
            installer->setSuccess(false);
        }
        installer->setInstalling(false);
    }

    void on_properties_changed(
        GDBusConnection *connection,
        const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name,
        GVariant *parameters, gpointer user_data)
    {
        if(nullptr != parameters) {
            const gchar  *iface;
            GVariantIter *properties = NULL;
            GVariantIter *unknown = NULL;
            const gchar  *key;
            GVariant     *value = NULL;

            g_variant_get(parameters, "(&sa{sv}as)", &iface, &properties, &unknown);
            if(!g_strcmp0(iface, "de.pengutronix.rauc.Installer")) {
                while(g_variant_iter_next(properties, "{&sv}", &key, &value))
                {
                    if(!g_strcmp0(key, "Operation")) {
                        const gchar *message;
                        g_variant_get(value, "s", &message);
                        Logger::trace("Rauc signals {}: {}", key, message);

                        DBusRaucInstaller* installer = (DBusRaucInstaller*)user_data;
                        if(!g_strcmp0(message, "installing")) {
                            installer->setSuccess(false);
                            installer->setInstalling(true);
                        }
                    }
                    else if(!g_strcmp0(key, "Compatible")) {
                        const gchar *message;
                        g_variant_get(value, "s", &message);
                        Logger::trace("Rauc signals {}: {}", key, message);

                        DBusRaucInstaller* installer = (DBusRaucInstaller*)user_data;
                        if(installer->installing()) {
                            Logger::error("Rauc service was re-started, installation failed");
                            installer->setSuccess(false);
                            installer->setInstalling(false);
                        }
                    }
                    else if(!g_strcmp0(key, "LastError") ||
                            !g_strcmp0(key, "Variant")   ||
                            !g_strcmp0(key, "BootSlot")    ) {
                        const gchar *message;
                        g_variant_get(value, "s", &message);
                        Logger::trace("Rauc signals {}: {}", key, message);
                    }
                    else if(!g_strcmp0(key, "Progress")) {
                        int32_t progressPercentage = 0;
                        gchar*  message;
                        int32_t nesting = 0;
                        g_variant_get(value, "(isi)", &progressPercentage, &message, &nesting);
                        Logger::trace("Rauc signals {}: {}% ({}) {}", key, progressPercentage, nesting, message);
                    }
                    else {
                        Logger::trace("Rauc signals {}", key);
                    }

                    g_variant_unref(value);
                }
            }
        }
    }

    DBusRaucInstaller::DBusRaucInstaller()
    {
        setupDBusRaucConnection();
        if(nullptr != connection) {
            subscribeDBusSignals();
        }
    }

    DBusRaucInstaller::~DBusRaucInstaller()
    {
        if(nullptr != connection) {
            unsubscribeDBusSignals();
            g_object_unref(connection);
        }

        if(nullptr != loop) {
            g_main_loop_unref(loop);
        }
    }

    TechCode DBusRaucInstaller::installBundle(const std::string& input)
    {
        g_main_context_iteration(g_main_loop_get_context(loop), FALSE);

        // check if RAUC is still installing
        std::string operation = getDBusRaucProperty("Operation");
        if (operation != "idle") {
            Logger::error("Rauc is not in operation 'idle'");
            return TechCode::InstallationFailed;
        }

        TechCode result = callDBusRaucInstallBundle(input);

        is_installing = (result == TechCode::OK);
        is_succeeded  = false;
        return result;
    }

    int32_t DBusRaucInstaller::getProgressPollInterval() const
    {
        return 2000;
    }

    int32_t DBusRaucInstaller::getInstallProgress()
    {
        return getDBusRaucInstallProgress();
    }

    SlotStatus DBusRaucInstaller::getSlotStatus()
    {
        auto slotStatus = getDBusRaucSlotStatus();
        g_main_context_iteration(g_main_loop_get_context(loop), FALSE);
        return slotStatus;
    }

    std::string DBusRaucInstaller::getBootedVersion()
    {
        SlotStatus  slotStatus    = getSlotStatus();
        std::string bootedVersion = "";

        for(auto it : slotStatus) {
            if(it.second["state"] == "booted") {
                if(!bootedVersion.empty()) {
                    return MULTIPLE_BOOTED_SLOTS;
                }

                bootedVersion = it.second["version"];
            }
        }

        if(bootedVersion == VERSION_UNAVAILABLE) {
            bootedVersion = getOsVersionId("EDITION_ID");

            if(bootedVersion == VERSION_UNAVAILABLE) {
                bootedVersion = getOsVersionId("VERSION_ID");
            }
        }

        return bootedVersion;
    }

    std::string DBusRaucInstaller::getBundleVersion(const std::string& input)
    {
        std::string bundleVersion = getDBusRaucBundleVersion(input);
        g_main_context_iteration(g_main_loop_get_context(loop), FALSE);

        return bundleVersion;
    }

    void DBusRaucInstaller::subscribeDBusSignals()
    {
        signalSubscriptionIdProperties = g_dbus_connection_signal_subscribe(
            connection,
            NULL,
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            "/",
            NULL,
            G_DBUS_SIGNAL_FLAGS_NONE,
            on_properties_changed,
            this,
            NULL);

        signalSubscriptionIdCompleted = g_dbus_connection_signal_subscribe(
            connection,
            NULL,
            "de.pengutronix.rauc.Installer",
            "Completed",
            "/",
            NULL,
            G_DBUS_SIGNAL_FLAGS_NONE,
            on_completed,
            this,
            NULL);
    }

    void DBusRaucInstaller::unsubscribeDBusSignals()
    {
        g_dbus_connection_signal_unsubscribe(connection, signalSubscriptionIdProperties);
        g_dbus_connection_signal_unsubscribe(connection, signalSubscriptionIdCompleted);
    }

    void DBusRaucInstaller::setupDBusRaucConnection()
    {
        GError* connectionError = nullptr;
        connection              = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &connectionError);
        loop                    = g_main_loop_new(NULL, FALSE);

        if(nullptr != connection) {
            Logger::trace("Valid connection to D-Bus");
        } else {
            Logger::error("Unable to connect to D-Bus, code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
        }
    }

    TechCode DBusRaucInstaller::callDBusRaucInstallBundle(const std::string& bundleName)
    {
        Logger::trace("DBusRaucInstaller::callDBusRaucInstallBundle");
        GError*         connectionError = nullptr;
        GVariantBuilder argumentBuilder;

        g_variant_builder_init(&argumentBuilder, G_VARIANT_TYPE("a{sv}"));  // array of arguments stays empty

        GVariant* result = g_dbus_connection_call_sync(
            connection,
            "de.pengutronix.rauc",
            "/",
            "de.pengutronix.rauc.Installer",
            "InstallBundle",
            g_variant_new("(sa{sv})", bundleName.c_str(), &argumentBuilder),
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &connectionError);

        if(nullptr != result) {
            g_variant_unref(result);
        } else {
            Logger::error("InstallBundle call to Rauc via D-Bus failed, code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
            return TechCode::InstallationFailed;
        }

        return TechCode::OK;
    }

    int32_t DBusRaucInstaller::getDBusRaucInstallProgress()
    {
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
            Logger::trace("Rauc Installer progress: {}% ({}) {}", progressPercentage, nesting, message);
            g_variant_unref(progressInfo);

            if(progressPercentage == 100 && nesting == 1) {
                if(!g_strcmp0(message, "Checking bundle done.")) {
                    // ignore a rare previous 100% progress before this installation started
                    progressPercentage = 0;
                }
                else if(g_strcmp0(message, "Installing done.")) {
                    // Installing failed.
                    setSuccess(false);
                    setInstalling(false);
                }
            }
        } else {
            Logger::error("Connection to D-Bus lost? code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
        }
        return progressPercentage;
    }

    std::string DBusRaucInstaller::getDBusRaucProperty(const gchar* propertyKey) const
    {
        GError*   connectionError = nullptr;
        GVariant* operationInfo   = g_dbus_connection_call_sync(
            connection,
            "de.pengutronix.rauc",
            "/",
            "org.freedesktop.DBus.Properties",
            "Get",
            g_variant_new("(ss)", "de.pengutronix.rauc.Installer", propertyKey),
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &connectionError);

        std::string propertyValue;
        if(nullptr != operationInfo) {
            GVariant* internalVal;
            gchar*    value;
            g_variant_get(operationInfo, "(v)", &internalVal);
            g_variant_get(internalVal, "s", &value);
            g_variant_unref(operationInfo);
            propertyValue = std::string(value);
        }
        else {
            Logger::error("Connection to D-Bus lost? code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
        }
        Logger::trace("Rauc Installer property {} = {}", propertyKey, propertyValue);
        return propertyValue;
    }

    SlotStatus DBusRaucInstaller::getDBusRaucSlotStatus() const
    {
        SlotStatus result;
        Logger::trace("DBusRaucInstaller::getDBusRaucSlotStatus"); 
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
            g_variant_get(slotStatus, "(a(sa{sv}))", &slotIter);
            while(g_variant_iter_next(slotIter, "(s@a{sv})", &slotName, &slotDict)) {
                gchar*       slotState;
                gchar*       bundle;
                GVariantDict dict;
                g_variant_dict_init(&dict, slotDict);
                if(g_variant_dict_lookup(&dict, "state", "s", &slotState)) {
                    result[slotName]["state"] = slotState;

                    if(g_variant_dict_lookup(&dict, "bundle.version", "s", &bundle)) {
                        result[slotName]["version"] = bundle;
                    } else {
                        result[slotName]["version"] = VERSION_UNAVAILABLE;
                    }
                } else {
                    result[slotName]["state"] = STATE_UNAVAILABLE;
                }
            }

            g_free(slotName);
            g_variant_unref(slotDict);
            g_variant_unref(slotStatus);
        }

        return result;
    }

    std::string DBusRaucInstaller::getOsVersionId(const std::string & version_key) const
    {
        Logger::trace("DBusRaucInstaller::getOsVersionId");
        std::string versionId = VERSION_UNAVAILABLE;
        std::ifstream file("/etc/os-release");
        if(file) {
            std::string keyValue;

            while(std::getline(file, keyValue)) {
                std::stringstream ss(keyValue);

                std::string key;
                std::getline(ss, key, '=');

                if(key == version_key) {
                    std::string value;
                    std::getline(ss, value, '=');
                    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
                    if(!value.empty()) {
                        versionId = value;
                        break;
                    }
                }
            }
        }

        return versionId;
    }

    std::string DBusRaucInstaller::getDBusRaucBundleVersion(const std::string& input) const
    {
        Logger::trace("DBusRaucInstaller::getDBusRaucBundleVersion out of {}", input);
        std::string bundleVersion   = VERSION_UNAVAILABLE;
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
            gchar* compatible;
            gchar* version;
            g_variant_get(result, "(ss)", &compatible, &version);
            bundleVersion = std::string(version);
            g_free(compatible);
            g_free(version);
        } else {
            Logger::trace("Retrieval of bundle version was not succesfull, error = {}",
                          connectionError->message);
        }

        Logger::trace("Retrieved version of the incoming bundle is: '{}'", bundleVersion);
        return bundleVersion;
    }

    TechCode DBusRaucInstaller::activateBooted()
    {
        return callDBusRaucMark("booted", "active");
    }

    TechCode DBusRaucInstaller::activateOther()
    {
        return callDBusRaucMark("other", "active");
    }

    TechCode DBusRaucInstaller::callDBusRaucMark(const std::string& identifier, const std::string& state)
    {
        Logger::trace("DBusRaucInstaller::callDBusRaucMark for slot '{}' to new state '{}'", identifier, state);

        GError* connectionError = nullptr;

        GVariant* result = g_dbus_connection_call_sync(
            connection,
            "de.pengutronix.rauc",
            "/",
            "de.pengutronix.rauc.Installer",
            "Mark",
            g_variant_new("(ss)", state.c_str(), identifier.c_str()),
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &connectionError);

        if(nullptr != result) {
            g_variant_unref(result);
        } else {
            Logger::error("Mark-active-slot call to Rauc via D-Bus failed, code = {}, message = {}",
                          connectionError->code,
                          connectionError->message);
            return TechCode::InstallationFailed;
        }

        return TechCode::OK;
    }

    std::string DBusRaucInstaller::getLastError()
    {
        std::string errorMessage{getDBusRaucProperty("LastError")};
        if (errorMessage.empty())
        {
            errorMessage = "Unexpected no-error-status of RAUC";
        }
        return errorMessage;
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

    void DBusRaucInstaller::setInstalling(const bool value)
    {
        is_installing = value;
    }

} // namespace sua
