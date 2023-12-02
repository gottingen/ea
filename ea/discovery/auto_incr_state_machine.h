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

#include <unordered_map>
#include "ea/discovery/base_state_machine.h"
#include "ea/discovery/discovery_constants.h"

namespace EA::discovery {
    class AutoIncrStateMachine : public EA::discovery::BaseStateMachine {
    public:

        explicit AutoIncrStateMachine(const braft::PeerId &peerId) :
                BaseStateMachine(DiscoveryConstants::AutoIDMachineRegion, "auto_incr_raft", "/auto_incr", peerId) {}

        ~AutoIncrStateMachine() override = default;

        /// state machine method override
        void on_apply(braft::Iterator &iter) override;

        ///
        /// \brief servlet inc id initialize
        /// \param request [in]
        /// \param done [out]
        void add_servlet_id(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief servlet inc id removing
        /// \param request [in]
        /// \param done [out]
        void drop_servlet_id(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief gen a servlet inc id by given count in request
        /// \param request [in]
        /// \param done [out]
        void gen_id(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief reset a servlet inc by start_id or increment_id, if backwards,
        ///        increment_info.force() should be enabled.
        /// \param request [in]
        /// \param done [out]
        void update(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief override BaseStateMachine::on_snapshot_save
        /// \param writer   braft snapshot writer.
        /// \param done
        void on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done) override;

        ///
        /// \brief override BaseStateMachine::on_snapshot_load
        /// \param writer   braft snapshot writer.
        /// \param done
        int on_snapshot_load(braft::SnapshotReader *reader)  override;

    private:
        void save_auto_increment(std::string &max_id_string);

        void save_snapshot(braft::Closure *done,
                           braft::SnapshotWriter *writer,
                           std::string max_id_string);
        ///
        /// \brief load json servlet_id --> max_id from json file
        /// \param max_id_file
        /// \return

        int load_auto_increment(const std::string &max_id_file);

        ///
        /// \param json_string
        /// \return
        int parse_json_string(const std::string &json_string);

        std::unordered_map<int64_t, uint64_t> _auto_increment_map;
    };

} //namespace EA::discovery

