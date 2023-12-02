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
// Created by jeff on 23-11-26.
//

#ifndef EA_BASE_SCOPE_EXIT_H_
#define EA_BASE_SCOPE_EXIT_H_

#include <functional>
#include "butil/macros.h"

namespace EA {

    class ScopeGuard {
    public:
        explicit ScopeGuard(std::function<void()> exit_func) :
                _exit_func(exit_func) {}

        ~ScopeGuard() {
            if (!_is_release) {
                _exit_func();
            }
        }

        void release() {
            _is_release = true;
        }

    private:
        std::function<void()> _exit_func;
        bool _is_release = false;
        DISALLOW_COPY_AND_ASSIGN(ScopeGuard);
    };

#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)
#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPEGUARD_LINENAME(scope_guard, __LINE__)(callback)
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p){delete(p);  (p)=nullptr;} }
#endif

}  // namespace EA
#endif  // EA_BASE_SCOPE_EXIT_H_
