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

#include "standby_hitrace_chain.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    constexpr int32_t DEFAULT_FLAGS = HitraceFlag::HITRACE_FLAG_INCLUDE_ASYNC |
                                      HitraceFlag::HITRACE_FLAG_DONOT_CREATE_SPAN |
                                      HitraceFlag::HITRACE_FLAG_NO_BE_INFO;
}
StandbyHitraceChain::StandbyHitraceChain(const bool isClearId, const char *name)
{
    if (isClearId) {
        HitraceChainClearId();
        isBegin_ = true;
        traceId_ = HitraceChainBegin(name, DEFAULT_FLAGS);
    }
}

StandbyHitraceChain::StandbyHitraceChain(const char *name, const int32_t flags)
{
    HitraceIdStruct currentId = HitraceChainGetId();
    isBegin_ = !HitraceChainIsValid(&currentId);
    if (isBegin_) {
        traceId_ = HitraceChainBegin(name, (flags > 0) ? flags : DEFAULT_FLAGS);
    }
}

StandbyHitraceChain::~StandbyHitraceChain()
{
    if (isBegin_) {
        HitraceChainEnd(&traceId_);
    }
}
}
}