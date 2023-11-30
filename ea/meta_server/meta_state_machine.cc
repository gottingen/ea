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


#include "ea/meta_server/meta_state_machine.h"
#include <braft/util.h>
#include <braft/storage.h>
#include "ea/base/scope_exit.h"
#include "ea/meta_server/privilege_manager.h"
#include "ea/meta_server/schema_manager.h"
#include "ea/meta_server/config_manager.h"
#include "ea/meta_server/namespace_manager.h"
#include "ea/meta_server/zone_manager.h"
#include "ea/meta_server/instance_manager.h"
#include "ea/meta_server/servlet_manager.h"
#include "ea/engine/rocks_storage.h"
#include "ea/meta_server/query_privilege_manager.h"
#include "ea/engine/sst_file_writer.h"
#include "ea/meta_server/parse_path.h"

namespace EA::servlet {


    void MetaStateMachine::on_apply(braft::Iterator &iter) {
        for (; iter.valid(); iter.next()) {
            braft::Closure *done = iter.done();
            brpc::ClosureGuard done_guard(done);
            if (done) {
                ((MetaServerClosure *) done)->raft_time_cost = ((MetaServerClosure *) done)->time_cost.get_time();
            }
            butil::IOBufAsZeroCopyInputStream wrapper(iter.data());
            EA::servlet::MetaManagerRequest request;
            if (!request.ParseFromZeroCopyStream(&wrapper)) {
                TLOG_ERROR("parse from protobuf fail when on_apply");
                if (done) {
                    if (((MetaServerClosure *) done)->response) {
                        ((MetaServerClosure *) done)->response->set_errcode(EA::servlet::PARSE_FROM_PB_FAIL);
                        ((MetaServerClosure *) done)->response->set_errmsg("parse from protobuf fail");
                    }
                    braft::run_closure_in_bthread(done_guard.release());
                }
                continue;
            }
            if (done && ((MetaServerClosure *) done)->response) {
                ((MetaServerClosure *) done)->response->set_op_type(request.op_type());
            }
            TLOG_INFO("on apply, term:{}, index:{}, request op_type:{}",
                      iter.term(), iter.index(),
                      EA::servlet::OpType_Name(request.op_type()));
            switch (request.op_type()) {
                case EA::servlet::OP_CREATE_USER: {
                    PrivilegeManager::get_instance()->create_user(request, done);
                    break;
                }
                case EA::servlet::OP_DROP_USER: {
                    PrivilegeManager::get_instance()->drop_user(request, done);
                    break;
                }
                case EA::servlet::OP_ADD_PRIVILEGE: {
                    PrivilegeManager::get_instance()->add_privilege(request, done);
                    break;
                }
                case EA::servlet::OP_DROP_PRIVILEGE: {
                    PrivilegeManager::get_instance()->drop_privilege(request, done);
                    break;
                }
                case EA::servlet::OP_CREATE_NAMESPACE: {
                    NamespaceManager::get_instance()->create_namespace(request, done);
                    break;
                }
                case EA::servlet::OP_DROP_NAMESPACE: {
                    NamespaceManager::get_instance()->drop_namespace(request, done);
                    break;
                }
                case EA::servlet::OP_MODIFY_NAMESPACE: {
                    NamespaceManager::get_instance()->modify_namespace(request, done);
                    break;
                }
                case EA::servlet::OP_CREATE_ZONE: {
                    ZoneManager::get_instance()->create_zone(request, done);
                    break;
                }
                case EA::servlet::OP_DROP_ZONE: {
                    ZoneManager::get_instance()->drop_zone(request, done);
                    break;
                }
                case EA::servlet::OP_MODIFY_ZONE: {
                    ZoneManager::get_instance()->modify_zone(request, done);
                    break;
                }
                case EA::servlet::OP_CREATE_SERVLET: {
                    ServletManager::get_instance()->create_servlet(request, done);
                    break;
                }
                case EA::servlet::OP_DROP_SERVLET: {
                    ServletManager::get_instance()->drop_servlet(request, done);
                    break;
                }
                case EA::servlet::OP_MODIFY_SERVLET: {
                    ServletManager::get_instance()->modify_servlet(request, done);
                    break;
                }
                case EA::servlet::OP_CREATE_CONFIG: {
                    ConfigManager::get_instance()->create_config(request, done);
                    break;
                }
                case EA::servlet::OP_REMOVE_CONFIG: {
                    ConfigManager::get_instance()->remove_config(request, done);
                    break;
                }
                case EA::servlet::OP_ADD_INSTANCE: {
                    InstanceManager::get_instance()->add_instance(request, done);
                    break;
                }
                case EA::servlet::OP_DROP_INSTANCE: {
                    InstanceManager::get_instance()->drop_instance(request, done);
                    break;
                }
                case EA::servlet::OP_UPDATE_INSTANCE: {
                    InstanceManager::get_instance()->update_instance(request, done);
                    break;
                }
                default: {
                    TLOG_ERROR("unknown request type, type:{}", request.op_type());
                    IF_DONE_SET_RESPONSE(done, EA::servlet::UNKNOWN_REQ_TYPE, "unknown request type");
                }
            }
            _applied_index = iter.index();
            if (done) {
                braft::run_closure_in_bthread(done_guard.release());
            }
        }
    }

    void MetaStateMachine::on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done) {
        TLOG_WARN("start on snapshot save");
        TLOG_WARN("max_namespace_id: {}, max_zone_id: {},"
                   " when on snapshot save",
                   NamespaceManager::get_instance()->get_max_namespace_id(),
                   ZoneManager::get_instance()->get_max_zone_id());
        //创建snapshot
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = false;
        read_options.total_order_seek = true;
        auto iter = RocksStorage::get_instance()->new_iterator(read_options,
                                                               RocksStorage::get_instance()->get_meta_info_handle());
        iter->SeekToFirst();
        Bthread bth(&BTHREAD_ATTR_SMALL);
        std::function<void()> save_snapshot_function = [this, done, iter, writer]() {
            save_snapshot(done, iter, writer);
        };
        bth.run(save_snapshot_function);
    }

    void MetaStateMachine::save_snapshot(braft::Closure *done,
                                         rocksdb::Iterator *iter,
                                         braft::SnapshotWriter *writer) {
        brpc::ClosureGuard done_guard(done);
        std::unique_ptr<rocksdb::Iterator> iter_lock(iter);

        std::string snapshot_path = writer->get_path();
        std::string sst_file_path = snapshot_path + "/meta_info.sst";

        rocksdb::Options option = RocksStorage::get_instance()->get_options(
                RocksStorage::get_instance()->get_meta_info_handle());
        SstFileWriter sst_writer(option);
        TLOG_WARN("snapshot path:{}", snapshot_path);
        //Open the file for writing
        auto s = sst_writer.open(sst_file_path);
        if (!s.ok()) {
            TLOG_WARN("Error while opening file {}, Error: {}", sst_file_path,
                       s.ToString());
            done->status().set_error(EINVAL, "Fail to open SstFileWriter");
            return;
        }
        for (; iter->Valid(); iter->Next()) {
            auto res = sst_writer.put(iter->key(), iter->value());
            if (!res.ok()) {
                TLOG_WARN("Error while adding Key: {}, Error: {}",
                           iter->key().ToString(),
                           s.ToString());
                done->status().set_error(EINVAL, "Fail to write SstFileWriter");
                return;
            }
        }
        //close the file
        s = sst_writer.finish();
        if (!s.ok()) {
            TLOG_WARN("Error while finishing file {}, Error: {}", sst_file_path,
                       s.ToString());
            done->status().set_error(EINVAL, "Fail to finish SstFileWriter");
            return;
        }
        if (writer->add_file("/meta_info.sst") != 0) {
            done->status().set_error(EINVAL, "Fail to add file");
            TLOG_WARN("Error while adding file to writer");
            return;
        }
    }

    int MetaStateMachine::on_snapshot_load(braft::SnapshotReader *reader) {
        TLOG_WARN("start on snapshot load");
        //先删除数据
        std::string remove_start_key(MetaConstants::SCHEMA_IDENTIFY);
        rocksdb::WriteOptions options;
        auto status = RocksStorage::get_instance()->remove_range(options,
                                                                 RocksStorage::get_instance()->get_meta_info_handle(),
                                                                 remove_start_key,
                                                                 MetaConstants::MAX_IDENTIFY,
                                                                 false);
        if (!status.ok()) {
            TLOG_ERROR("remove_range error when on snapshot load: code={}, msg={}",
                     status.code(), status.ToString());
            return -1;
        } else {
            TLOG_WARN("remove range success when on snapshot load:code:{}, msg={}",
                       status.code(), status.ToString());
        }
        TLOG_WARN("clear data success");
        rocksdb::ReadOptions read_options;
        std::unique_ptr<rocksdb::Iterator> iter(RocksStorage::get_instance()->new_iterator(read_options,
                                                                                           RocksStorage::get_instance()->get_meta_info_handle()));
        iter->Seek(MetaConstants::SCHEMA_IDENTIFY);
        for (; iter->Valid(); iter->Next()) {
            TLOG_WARN("iter key:{}, iter value:{} when on snapshot load",
                       iter->key().ToString(), iter->value().ToString());
        }
        std::vector<std::string> files;
        reader->list_files(&files);
        for (auto &file: files) {
            TLOG_WARN("snapshot load file:{}", file);
            if (file == "/meta_info.sst") {
                std::string snapshot_path = reader->get_path();
                _applied_index = parse_snapshot_index_from_path(snapshot_path, false);
                TLOG_WARN("_applied_index:{} path:{}", _applied_index, snapshot_path);
                snapshot_path.append("/meta_info.sst");

                //恢复文件
                rocksdb::IngestExternalFileOptions ifo;
                auto res = RocksStorage::get_instance()->ingest_external_file(
                        RocksStorage::get_instance()->get_meta_info_handle(),
                        {snapshot_path},
                        ifo);
                if (!res.ok()) {
                    TLOG_WARN("Error while ingest file {}, Error {}",
                               snapshot_path, res.ToString());
                    return -1;

                }
                auto ret = PrivilegeManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    TLOG_ERROR("PrivilegeManager load snapshot fail");
                    return -1;
                }
                ret = SchemaManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    TLOG_ERROR("SchemaManager load snapshot fail");
                    return -1;
                }

                ret = ConfigManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    TLOG_ERROR("ConfigManager load snapshot fail");
                    return -1;
                }
                ret = InstanceManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    TLOG_ERROR("Instance load snapshot fail");
                    return -1;
                }
            }
        }
        set_have_data(true);
        return 0;
    }

    void MetaStateMachine::on_leader_start() {
        TLOG_WARN("leader start at new term");
        BaseStateMachine::on_leader_start();
        _is_leader.store(true);
    }

    void MetaStateMachine::on_leader_stop() {
        _is_leader.store(false);
        TLOG_WARN("leader stop");
        BaseStateMachine::on_leader_stop();
    }

}  // namespace EA::servlet
