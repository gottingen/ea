// Copyright 2023 The Elastic-AI Authors.
// part of Elastic AI Search
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//
// Created by jeff on 23-11-19.
//

#ifndef EA_BASE_BTHREAD_H_
#define EA_BASE_BTHREAD_H_

#include "ea/base/tlog.h"
#include <bthread/butex.h>
#include <bthread/bthread.h>
#include <bthread/unstable.h>

namespace EA {

    // return when timeout or shutdown
    template<typename T>
    inline void bthread_usleep_fast_shutdown(int64_t interval_us, T &shutdown) {
        if (interval_us < 10000) {
            bthread_usleep(interval_us);
            return;
        }
        int64_t sleep_time_count = interval_us / 10000; //10ms为单位
        int time = 0;
        while (time < sleep_time_count) {
            if (shutdown) {
                return;
            }
            bthread_usleep(10000);
            ++time;
        }
    }

    // wrapper bthread functions for c++ style
    class Bthread {
    public:
        Bthread() {
        }

        explicit Bthread(const bthread_attr_t *attr) : _attr(attr) {
        }

        void run(const std::function<void()> &call) {
            std::function<void()> *_call = new std::function<void()>;
            *_call = call;
            int ret = bthread_start_background(&_tid, _attr,
                                               [](void *p) -> void * {
                                                   auto call = static_cast<std::function<void()> *>(p);
                                                   (*call)();
                                                   delete call;
                                                   return nullptr;
                                               }, _call);
            if (ret != 0) {
                TLOG_ERROR("bthread_start_background fail");
            }
        }

        void run_urgent(const std::function<void()> &call) {
            std::function<void()> *_call = new std::function<void()>;
            *_call = call;
            int ret = bthread_start_urgent(&_tid, _attr,
                                           [](void *p) -> void * {
                                               auto call = static_cast<std::function<void()> *>(p);
                                               (*call)();
                                               delete call;
                                               return nullptr;
                                           }, _call);
            if (ret != 0) {
                TLOG_ERROR("bthread_start_urgent fail");
            }
        }

        void join() {
            bthread_join(_tid, nullptr);
        }

        bthread_t id() {
            return _tid;
        }

    private:
        bthread_t _tid;
        const bthread_attr_t *_attr = nullptr;
    };

}  // namespace EA

#endif  // EA_BASE_BTHREAD_H_
