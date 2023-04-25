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

#ifndef SDV_SUA_LOGGER_H
#define SDV_SUA_LOGGER_H

#include <spdlog/spdlog.h>

namespace sua {

    class Logger {
    public:
        enum Level {
            None     = 0x0000,
            Trace    = 0x0001,
            Debug    = 0x0002,
            Info     = 0x0004,
            Warning  = 0x0008,
            Error    = 0x0010,
            Critical = 0x0020,
            All      = 0xFFFF
        };

        Logger();

        void init();
        void shutdown();

        void setLogLevel(int mask);
        int  getLogLevel() const;

        template <typename... Args>
        static void trace(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            if(instance().getLogLevel() & Level::Trace) {
                spdlog::trace(fmt, std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        static void debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            if(instance().getLogLevel() & Level::Debug) {
                spdlog::debug(fmt, std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        static void info(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            if(instance().getLogLevel() & Level::Info) {
                spdlog::info(fmt, std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        static void warning(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            if(instance().getLogLevel() & Level::Warning) {
                spdlog::warn(fmt, std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        static void error(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            if(instance().getLogLevel() & Level::Error) {
                spdlog::error(fmt, std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        static void critical(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            if(instance().getLogLevel() & Level::Critical) {
                spdlog::critical(fmt, std::forward<Args>(args)...);
            }
        }

    public:
        static Logger& instance();

    private:
        int _log_level;
    };

} // namespace sua

#endif
