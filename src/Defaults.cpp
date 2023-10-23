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

#include "Defaults.h"

const std::string SUA_DEFAULT_MQTT_PROTOCOL  = "tcp";
const std::string SUA_DEFAULT_MQTT_HOST      = "mosquitto";
const int         SUA_DEFAULT_MQTT_PORT      = 1883;
const std::string SUA_DEFAULT_MQTT_SERVER    = "tcp://mosquitto:1883";
const std::string SUA_DEFAULT_MODE           = "download";
const std::string SUA_DEFAULT_TEMP_DIRECTORY = "/data/selfupdates";
const std::string SUA_DEFAULT_CA_DIRECTORY   = "/etc/ssl/certs";
const std::string SUA_DEFAULT_CA_FILEPATH    = "";
const int         SUA_DEFAULT_MESSAGE_DELAY  = 5;
