// Copyright 2023 The Turbo Authors.
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

#include "ea/cli/validator.h"
#include "turbo/container/flat_hash_set.h"

namespace EA::cli {
    static turbo::flat_hash_set<char> AllowChar{'a', 'b', 'c', 'd', 'e', 'f', 'g',
                                                'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                                'o', 'p', 'q', 'r', 's', 't',
                                                'u', 'v', 'w', 'x', 'y', 'z',
                                                '0','1','2','3','4','5','6','7','8','9',
                                                '_',
                                                'A', 'B', 'C', 'D', 'E', 'F', 'G',
                                                'H', 'I', 'J', 'K', 'L', 'M', 'N',
                                                'O', 'P', 'Q', 'R', 'S', 'T',
                                                'U', 'V', 'Q', 'X', 'Y', 'Z',
    };

    turbo::Status CheckValidNameType(std::string_view ns) {
        int i = 0;
        for(auto c : ns) {
            if(AllowChar.find(c) == AllowChar.end()) {
                return turbo::InvalidArgumentError("the {} char {} of {} is not allow used in name the valid set is[a-z,A-Z,0-9,_]", i, c, ns);
            }
            ++i;
        }
        return turbo::OkStatus();
    }
}  // namespace EA::cli
