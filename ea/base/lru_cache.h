// Copyright 2023 The Elastic AI Search Authors.
//
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


#ifndef EA_BASE_LRU_CACHE_H_
#define EA_BASE_LRU_CACHE_H_

#include <sys/time.h>
#include <string>
#include <unordered_map>
#include <mutex>
#include <butil/containers/linked_list.h>

namespace EA {

    template<typename ItemKey, typename ItemType>
    struct LruNode : public butil::LinkNode<LruNode<ItemKey, ItemType>> {
        ItemType value;
        ItemKey key;
    };

    template<typename ItemKey, typename ItemType>
    class Cache {
    public:
        Cache() : _total_count(0), _hit_count(0),
                  _len_threshold(10000) {}

        int init(int64_t len_threshold);

        std::string get_info();

        int check(const ItemKey &key);

        int find(const ItemKey &key, ItemType *value);

        int add(const ItemKey &key, const ItemType &value);

        int del(const ItemKey &key);

    private:
        //双链表，从尾部插入数据，超过阈值数据从头部删除
        butil::LinkedList<LruNode<ItemKey, ItemType>> _lru_list;
        std::unordered_map<ItemKey, LruNode<ItemKey, ItemType> *> _lru_map;
        std::mutex _mutex;
        int64_t _total_count;
        int64_t _hit_count;
        int64_t _len_threshold;
    };

    template<typename ItemKey, typename ItemType>
    int Cache<ItemKey, ItemType>::init(int64_t len_threshold) {
        _len_threshold = len_threshold;
        return 0;
    }

    template<typename ItemKey, typename ItemType>
    std::string Cache<ItemKey, ItemType>::get_info() {
        char buf[100];
        snprintf(buf, sizeof(buf), "hit:%ld, total:%ld,", _hit_count, _total_count);
        return buf;
    }

    template<typename ItemKey, typename ItemType>
    int Cache<ItemKey, ItemType>::check(const ItemKey &key) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_lru_map.count(key) == 1) {
            return 0;
        }
        return -1;
    }

    template<typename ItemKey, typename ItemType>
    int Cache<ItemKey, ItemType>::find(const ItemKey &key, ItemType *value) {
        std::lock_guard<std::mutex> lock(_mutex);
        ++_total_count;
        if (_lru_map.count(key) == 1) {
            ++_hit_count;
            LruNode<ItemKey, ItemType> *node = _lru_map[key];
            *value = node->value;
            node->RemoveFromList();
            _lru_list.Append(node);
            return 0;
        }
        return -1;
    }

    template<typename ItemKey, typename ItemType>
    int Cache<ItemKey, ItemType>::add(const ItemKey &key, const ItemType &value) {
        std::lock_guard<std::mutex> lock(_mutex);
        LruNode<ItemKey, ItemType> *node = nullptr;
        if (_lru_map.count(key) == 1) {
            node = _lru_map[key];
            node->RemoveFromList();
        } else {
            node = new LruNode<ItemKey, ItemType>();
            _lru_map[key] = node;
        }
        while ((int64_t) _lru_map.size() >= _len_threshold && !_lru_list.empty()) {
            LruNode<ItemKey, ItemType> *head = (LruNode<ItemKey, ItemType> *) _lru_list.head();
            head->RemoveFromList();
            _lru_map.erase(head->key);
            delete head;
        }
        node->value = value;
        node->key = key;
        _lru_list.Append(node);
        return 0;
    }

    template<typename ItemKey, typename ItemType>
    int Cache<ItemKey, ItemType>::del(const ItemKey &key) {
        std::lock_guard<std::mutex> lock(_mutex);
        LruNode<ItemKey, ItemType> *node = nullptr;
        if (_lru_map.count(key) == 1) {
            node = _lru_map[key];
            node->RemoveFromList();
            _lru_map.erase(node->key);
        }
        return 0;
    }

}  // namespace EA

#endif  // EA_BASE_LRU_CACHE_H_
