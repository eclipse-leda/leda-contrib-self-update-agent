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

#ifndef SDV_SUA_DISPATCHER_H
#define SDV_SUA_DISPATCHER_H

#include <functional>
#include <string>
#include <vector>
#include <map>

namespace sua {

    class Dispatcher {
    public:
        using Callback = std::function<void(const std::map<std::string, std::string>&)>;

        int  subscribe(const std::string& topic, Callback callback);
        void unsubscribe(int id);

        void dispatch(const std::string& topic, const std::map<std::string, std::string>& payload);

    public:
        static Dispatcher& instance();

    private:
        struct Subscriber {
            int         id;
            std::string topic;
            Callback    callback;
        };

        std::vector<Subscriber> _subscribers;
    };

    class DispatcherSubscriber {
    public:
        DispatcherSubscriber() = default;
        virtual ~DispatcherSubscriber();

        void subscribe(const std::string& topic, Dispatcher::Callback callback);

    private:
        std::vector<int> _subscriptions;
    };

} // namespace sua

#endif
