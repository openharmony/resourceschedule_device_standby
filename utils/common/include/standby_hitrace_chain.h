/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_STANDBY_HITRACE_CHAIN_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_STANDBY_HITRACE_CHAIN_H

#include "hitracechainc.h"

namespace OHOS {
namespace DevStandbyMgr {
class StandbyHitraceChain {
public:
    /**
    * start a new HitraceChain
    */
    StandbyHitraceChain(const bool isClearId, const char *name);

    /**
    * get current HitraceChain
    */
    explicit StandbyHitraceChain(const char *name, const int32_t flags = -1);
    ~StandbyHitraceChain();

private:
    bool isBegin_ = false;
    HitraceIdStruct traceId_ = {0};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_STANDBY_HITRACE_CHAIN_H