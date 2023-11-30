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


#pragma once

#include "rocksdb/listener.h"
#include "ea/engine/rocks_storage.h"
#include "ea/base/tlog.h"

namespace EA {
class SimpleListener : public rocksdb::EventListener {
public:
    virtual ~SimpleListener() {}
    virtual void OnStallConditionsChanged(const rocksdb::WriteStallInfo& info) {
        bool is_stall = info.condition.cur != rocksdb::WriteStallCondition::kNormal;
        TLOG_INFO("OnStallConditionsChanged, cf:{} is_stall:{}", info.cf_name, is_stall);
    }
    virtual void OnFlushCompleted(rocksdb::DB* /*db*/, const rocksdb::FlushJobInfo& info) {
        uint64_t file_number = info.file_number;
        RocksStorage::get_instance()->set_flush_file_number(info.cf_name, file_number);
        TLOG_INFO("OnFlushCompleted, cf:{} file_number:{}", info.cf_name, file_number);
    }
    virtual void OnExternalFileIngested(rocksdb::DB* /*db*/, const rocksdb::ExternalFileIngestionInfo& info) {
        TLOG_INFO("OnExternalFileIngested, cf:{} table_properties:{}",
                info.cf_name, info.table_properties.ToString());
    }
};
}
