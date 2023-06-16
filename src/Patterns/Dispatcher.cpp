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

#include "Patterns/Dispatcher.h"

#include <algorithm>

namespace sua {

    int Dispatcher::subscribe(const std::string& topic, Callback callback)
    {
        static int id = 0;
        _subscribers.push_back({id, topic, callback});
        return id++;
    }

    void Dispatcher::unsubscribe(const int id)
    {
        _subscribers.erase(std::remove_if(_subscribers.begin(),
                                          _subscribers.end(),
                                          [id](const Subscriber& s) { return s.id == id; }),
                           _subscribers.end());
    }

    void Dispatcher::dispatch(const std::string& topic, const std::map<std::string, std::string>& payload)
    {
        for(const auto& s : _subscribers) {
            if(s.topic == topic) {
                s.callback(payload);
            }
        }
    }

    Dispatcher& Dispatcher::instance()
    {
        static Dispatcher d;
        return d;
    }

    DispatcherSubscriber::~DispatcherSubscriber()
    {
        for(const auto id : _subscriptions) {
            Dispatcher::instance().unsubscribe(id);
        }
    }

    void DispatcherSubscriber::subscribe(const std::string& topic, Dispatcher::Callback callback)
    {
        _subscriptions.push_back(Dispatcher::instance().subscribe(topic, callback));
    }

} // namespace sua
