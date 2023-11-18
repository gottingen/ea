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


#ifndef EA_BASE_TLOG_H_
#define EA_BASE_TLOG_H_

#include <initializer_list>
#include "ea/gflags/tlog.h"
#include "turbo/log/sinks/rotating_file_sink.h"
#include "turbo/log/sinks/daily_file_sink.h"
#include "turbo/log/sinks/stdout_color_sinks.h"
#include "turbo/files/filesystem.h"
#include "ea/base/proto_helper.h"
#include "turbo/log/logging.h"

namespace EA {
    inline bool init_tlog() {
        if (!turbo::filesystem::exists(FLAGS_log_root)) {
            if (!turbo::filesystem::create_directories(FLAGS_log_root)) {
                return false;
            }
        }
        turbo::filesystem::path lpath(FLAGS_log_root);
        lpath /= FLAGS_log_base_name;
        turbo::tlog::sink_ptr file_sink = std::make_shared<turbo::tlog::sinks::daily_file_sink_mt>(lpath.string(),
                                                                                  FLAGS_log_rotation_hour,
                                                                                  FLAGS_log_rotation_minute,
                                                                                  false, FLAGS_log_save_days);
        file_sink->set_level(turbo::tlog::level::trace);


        if(!FLAGS_enable_console_log) {
            auto logger = std::make_shared<turbo::tlog::logger>("ea-logger", file_sink);
            logger->set_level(turbo::tlog::level::debug);
            turbo::tlog::set_default_logger(logger);
        } else {
            turbo::tlog::sink_ptr console_sink = std::make_shared<turbo::tlog::sinks::stdout_color_sink_mt>();
            auto logger = std::make_shared<turbo::tlog::logger>("ea-logger", turbo::tlog::sinks_init_list{file_sink, console_sink});
            logger->set_level(turbo::tlog::level::debug);
            turbo::tlog::set_default_logger(logger);
        }
        return true;
    }
}
#endif  // EA_BASE_TLOG_H_
