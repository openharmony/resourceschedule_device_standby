/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "allow_type.h"

namespace OHOS {
namespace DevStandbyMgr {
std::vector<std::string> AllowTypeName = {
    "NETWORK",
    "RUNNING_LOCK",
    "TIMER",
    "WORK_SCHEDULER",
    "AUTO_SYNC",
    "PUSH",
    "FREEZE"
};
const uint32_t MAX_ALLOW_TYPE_NUM = AllowTypeName.size();
const uint32_t MAX_ALLOW_TYPE_NUMBER = (1 << MAX_ALLOW_TYPE_NUM) - 1;
}  // namespace DevStandbyMgr
}  // namespace OHOS