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
// Created by jeff on 23-11-25.
//

#ifndef EA_BASE_BTHREAD_H_
#define EA_BASE_BTHREAD_H_

#include "ea/base/tlog.h"
#include "bthread/bthread.h"

namespace EA {

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

    class BthreadCond {
    public:
        BthreadCond(int count = 0) {
            bthread_cond_init(&_cond, nullptr);
            bthread_mutex_init(&_mutex, nullptr);
            _count = count;
        }

        ~BthreadCond() {
            bthread_mutex_destroy(&_mutex);
            bthread_cond_destroy(&_cond);
        }

        int count() const {
            return _count;
        }

        void increase() {
            bthread_mutex_lock(&_mutex);
            ++_count;
            bthread_mutex_unlock(&_mutex);
        }

        void decrease_signal() {
            bthread_mutex_lock(&_mutex);
            --_count;
            bthread_cond_signal(&_cond);
            bthread_mutex_unlock(&_mutex);
        }

        void decrease_broadcast() {
            bthread_mutex_lock(&_mutex);
            --_count;
            bthread_cond_broadcast(&_cond);
            bthread_mutex_unlock(&_mutex);
        }

        int wait(int cond = 0) {
            int ret = 0;
            bthread_mutex_lock(&_mutex);
            while (_count > cond) {
                ret = bthread_cond_wait(&_cond, &_mutex);
                if (ret != 0) {
                    TLOG_WARN("wait timeout, ret:{}", ret);
                    break;
                }
            }
            bthread_mutex_unlock(&_mutex);
            return ret;
        }

        int increase_wait(int cond = 0) {
            int ret = 0;
            bthread_mutex_lock(&_mutex);
            while (_count + 1 > cond) {
                ret = bthread_cond_wait(&_cond, &_mutex);
                if (ret != 0) {
                    TLOG_WARN("wait timeout, ret:{}", ret);
                    break;
                }
            }
            ++_count; // 不能放在while前面
            bthread_mutex_unlock(&_mutex);
            return ret;
        }

        int timed_wait(int64_t timeout_us, int cond = 0) {
            int ret = 0;
            timespec tm = butil::microseconds_from_now(timeout_us);
            bthread_mutex_lock(&_mutex);
            while (_count > cond) {
                ret = bthread_cond_timedwait(&_cond, &_mutex, &tm);
                if (ret != 0) {
                    TLOG_WARN("wait timeout, ret:{}", ret);
                    break;
                }
            }
            bthread_mutex_unlock(&_mutex);
            return ret;
        }

        int increase_timed_wait(int64_t timeout_us, int cond = 0) {
            int ret = 0;
            timespec tm = butil::microseconds_from_now(timeout_us);
            bthread_mutex_lock(&_mutex);
            while (_count + 1 > cond) {
                ret = bthread_cond_timedwait(&_cond, &_mutex, &tm);
                if (ret != 0) {
                    TLOG_WARN("wait timeout, ret: {}", ret);
                    break;
                }
            }
            ++_count;
            bthread_mutex_unlock(&_mutex);
            return ret;
        }

    private:
        int _count;
        bthread_cond_t _cond;
        bthread_mutex_t _mutex;
    };

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

    class ConcurrencyBthread {
    public:
        explicit ConcurrencyBthread(int concurrency) :
                _concurrency(concurrency) {
        }

        ConcurrencyBthread(int concurrency, const bthread_attr_t *attr) :
                _concurrency(concurrency),
                _attr(attr) {
        }

        void run(const std::function<void()> &call) {
            _cond.increase_wait(_concurrency);
            Bthread bth(_attr);
            bth.run([this, call]() {
                call();
                _cond.decrease_signal();
            });
        }

        void join() {
            _cond.wait();
        }

        int count() const {
            return _cond.count();
        }

    private:
        int _concurrency = 10;
        BthreadCond _cond;
        const bthread_attr_t *_attr = nullptr;
    };

}  // namespace EA

#endif  // EA_BASE_BTHREAD_H_
