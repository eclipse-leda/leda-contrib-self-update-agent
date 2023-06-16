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

#include "Logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace sua {

    Logger& Logger::instance()
    {
        static Logger logger;
        return logger;
    }

    Logger::Logger()
    {
        _log_level = Level::All;
    }

    void Logger::init()
    {
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdout_sink->set_level(spdlog::level::trace);
        stdout_sink->set_pattern("%^%Y-%m-%dT%H:%M:%S.%f%z %-8l %v%$");

        auto json_file_sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("../logs/log.json", true);
        json_file_sink->set_level(spdlog::level::trace);
        json_file_sink->set_pattern(
            "{\"time\": \"%Y-%m-%dT%H:%M:%S.%f%z\", \"level\": \"%^%l%$\", \"message\": \"%v\"},");

        auto logger = std::make_shared<spdlog::logger>(
            "sua_logger", spdlog::sinks_init_list{stdout_sink, json_file_sink});
        logger->set_level(spdlog::level::trace);
        spdlog::set_default_logger(logger);
    }

    void Logger::shutdown()
    {
        spdlog::shutdown();
    }

    void Logger::setLogLevel(int mask)
    {
        _log_level = mask;
    }

    int Logger::getLogLevel() const
    {
        return _log_level;
    }

} // namespace sua
