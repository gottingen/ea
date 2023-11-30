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

#include <rocksdb/db.h>
#include "ea/meta_server/base_state_machine.h"
#include "eapi/servlet/servlet.interface.pb.h"
#include "ea/flags/meta.h"
#include "ea/meta_server/meta_constants.h"

namespace EA::servlet {
    class MetaStateMachine : public BaseStateMachine {
    public:
        MetaStateMachine(const braft::PeerId &peerId) :
                BaseStateMachine(MetaConstants::MetaMachineRegion, FLAGS_meta_raft_group, "/meta_server", peerId) {
        }

        ~MetaStateMachine() override = default;

        // state machine method
        void on_apply(braft::Iterator &iter) override;

        void on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done) override;

        int on_snapshot_load(braft::SnapshotReader *reader) override;

        void on_leader_start() override;

        void on_leader_stop() override;

        int64_t applied_index() { return _applied_index; }

    private:
        void save_snapshot(braft::Closure *done,
                           rocksdb::Iterator *iter,
                           braft::SnapshotWriter *writer);

        int64_t _applied_index = 0;
    };

}  // namespace EA::servlet

