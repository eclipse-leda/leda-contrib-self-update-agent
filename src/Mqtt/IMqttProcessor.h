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

#ifndef SDV_SUA_IMQTTPROCESSOR_H
#define SDV_SUA_IMQTTPROCESSOR_H

#include <string>

namespace sua {

    class IMqttProcessor {
    public:
        static constexpr const char * TOPIC_START     = "selfupdate/desiredstate";
        static constexpr const char * TOPIC_FEEDBACK  = "selfupdate/desiredstatefeedback";
        static constexpr const char * TOPIC_STATE     = "selfupdate/currentstate";
        static constexpr const char * TOPIC_STATE_GET = "selfupdate/currentstate/get";

        virtual ~IMqttProcessor() = default;

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual void send(const std::string& topic, const std::string& content, bool retained = false) = 0;
    };

} // namespace sua

#endif
