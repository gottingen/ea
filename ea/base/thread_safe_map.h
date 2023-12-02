// Copyright 2023 The Elastic Architecture Infrastructure Authors.
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

#ifndef EA_BASE_THREAD_SAFE_MAP_H_
#define EA_BASE_THREAD_SAFE_MAP_H_

#include <cstdint>
#include <unordered_map>
#include <functional>
#include <bthread/mutex.h>

namespace EA {

    template<typename KEY, typename VALUE, uint32_t MAP_COUNT = 23>
    class ThreadSafeMap {
        static_assert(MAP_COUNT > 0, "Invalid MAP_COUNT parameters.");
    public:
        ThreadSafeMap() {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                bthread_mutex_init(&_mutex[i], nullptr);
            }
        }

        ~ThreadSafeMap() {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                bthread_mutex_destroy(&_mutex[i]);
            }
        }

        uint32_t count(const KEY &key) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            return _map[idx].count(key);
        }

        uint32_t size() {
            uint32_t size = 0;
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                BAIDU_SCOPED_LOCK(_mutex[i]);
                size += _map[i].size();
            }
            return size;
        }

        void set(const KEY &key, const VALUE &value) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            _map[idx][key] = value;
        }

        // 已存在则不插入，返回false；不存在则init
        // init函数需要返回0，否则整个insert返回false
        bool insert_init_if_not_exist(const KEY &key, const std::function<int(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                if (call(_map[idx][key]) == 0) {
                    return true;
                } else {
                    _map[idx].erase(key);
                    return false;
                }
            } else {
                return false;
            }
        }

        const VALUE get(const KEY &key) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                static VALUE tmp;
                return tmp;
            }
            return _map[idx][key];
        }

        bool call_and_get(const KEY &key, const std::function<void(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                return false;
            } else {
                call(_map[idx][key]);
            }
            return true;
        }

        const VALUE get_or_put(const KEY &key, const VALUE &value) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                _map[idx][key] = value;
                return value;
            }
            return _map[idx][key];
        }

        const VALUE get_or_put_call(const KEY &key, const std::function<VALUE(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                return call(_map[idx][key]);
            }
            return _map[idx][key];
        }

        VALUE &operator[](const KEY &key) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            return _map[idx][key];
        }

        bool exist(const KEY &key) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            return _map[idx].count(key) > 0;
        }

        size_t erase(const KEY &key) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            return _map[idx].erase(key);
        }

        bool call_and_erase(const KEY &key, const std::function<void(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                return false;
            } else {
                call(_map[idx][key]);
                _map[idx].erase(key);
            }
            return true;
        }

        // 会加锁，轻量级操作采用traverse否则用copy
        void traverse(const std::function<void(VALUE &value)> &call) {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                BAIDU_SCOPED_LOCK(_mutex[i]);
                for (auto &pair: _map[i]) {
                    call(pair.second);
                }
            }
        }

        void traverse_with_key_value(const std::function<void(const KEY &key, VALUE &value)> &call) {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                BAIDU_SCOPED_LOCK(_mutex[i]);
                for (auto &pair: _map[i]) {
                    call(pair.first, pair.second);
                }
            }
        }

        void traverse_copy(const std::function<void(VALUE &value)> &call) {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                std::unordered_map<KEY, VALUE> tmp;
                {
                    BAIDU_SCOPED_LOCK(_mutex[i]);
                    tmp = _map[i];
                }
                for (auto &pair: tmp) {
                    call(pair.second);
                }
            }
        }

        void clear() {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                BAIDU_SCOPED_LOCK(_mutex[i]);
                _map[i].clear();
            }
        }

        // 已存在返回true，不存在init则返回false
        template<typename... Args>
        bool init_if_not_exist_else_update(const KEY &key, bool always_update,
                                           const std::function<void(VALUE &value)> &call, Args &&... args) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            auto iter = _map[idx].find(key);
            if (iter == _map[idx].end()) {
                _map[idx].insert(std::make_pair(key, VALUE(std::forward<Args>(args)...)));
                if (always_update) {
                    call(_map[idx][key]);
                }
                return false;
            } else {
                //字段存在，才执行回调
                call(iter->second);
                return true;
            }
        }

        bool update(const KEY &key, const std::function<void(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            BAIDU_SCOPED_LOCK(_mutex[idx]);
            auto iter = _map[idx].find(key);
            if (iter != _map[idx].end()) {
                call(iter->second);
                return true;
            } else {
                return false;
            }
        }

        //返回值：true表示执行了全部遍历，false表示遍历中途退出
        bool traverse_with_early_return(const std::function<bool(VALUE &value)> &call) {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                BAIDU_SCOPED_LOCK(_mutex[i]);
                for (auto &pair: _map[i]) {
                    if (!call(pair.second)) {
                        return false;
                    }
                }
            }
            return true;
        }

    private:
        uint32_t map_idx(const KEY &key) {
            return std::hash<KEY>{}(key) % MAP_COUNT;
        }

    private:
        std::unordered_map<KEY, VALUE> _map[MAP_COUNT];
        bthread_mutex_t _mutex[MAP_COUNT];
        DISALLOW_COPY_AND_ASSIGN(ThreadSafeMap);
    };
}  // namespace EA

#endif  // EA_BASE_THREAD_SAFE_MAP_H_
