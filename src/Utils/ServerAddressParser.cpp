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

#include "ServerAddressParser.h"

#include "spdlog/fmt/fmt.h"

#include <regex>
#include <algorithm>

namespace sua {

    void ServerAddressParser::parse(const std::string & address, std::string & transport, std::string & host, int & port)
    {
        std::regex  regex("^([a-zA-Z]+)\\:\\/\\/([^:\\/?#]+)(?:\\:([0-9]+))?$");
        std::smatch result;

        std::regex_match(address, result, regex);

        // 4 total because:
        // [0]         is entire match (address)
        // [1],[2],[3] are transport,host,port
        if(result.size() != 4) {
            throw std::runtime_error(fmt::format("Invalid server address: {}", address));
        }

        if(result[1].str().empty()) {
            throw std::runtime_error("Missing transport");
        }
        transport = result[1];
        std::transform(transport.begin(), transport.end(), transport.begin(), ::tolower);

        if(result[2].str().empty()) {
            throw std::runtime_error("Missing host");
        }
        host = result[2];

        if(result[3].str().empty()) {
            throw std::runtime_error("Missing port");
        }
        port = std::stoi(result[3]);
    }

} // namespace sua
