/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ipc_skeleton.h"
#include "time_service_client.h"
#include "accesstoken_kit.h"
#include "mock_ipc.h"

namespace {
    bool g_mockStartTimer = true;
    bool g_mockGetTokenTypeFlag = true;
}

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
int32_t IPCSkeleton::GetCallingUid()
{
    int32_t uid = 1;
    return uid;
}

int32_t IPCSkeleton::GetCallingPid()
{
    int32_t pid = 1;
    return pid;
}

namespace MiscServices {
bool TimeServiceClient::StartTimer(uint64_t timerId, uint64_t triggerTime)
{
    return g_mockStartTimer;
}

bool TimeServiceClient::StopTimer(uint64_t timerId)
{
    return g_mockStartTimer;
}

uint64_t TimeServiceClient::CreateTimer(std::shared_ptr<ITimerInfo> TimerOptions)
{
    return g_mockStartTimer;
}

bool TimeServiceClient::DestroyTimer(uint64_t timerId)
{
    return g_mockStartTimer;
}
} // namespace MiscServices

namespace Security {
namespace AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    if (g_mockGetTokenTypeFlag) {
        return TypeATokenTypeEnum::TOKEN_SHELL;
    }
    return TypeATokenTypeEnum::TOKEN_HAP;
}
}
}

namespace DevStandbyMgr {
void MockIpc::MockStartTimer(bool mockRet)
{
    g_mockStartTimer = mockRet;
}

void MockIpc::MockGetTokenTypeFlag(bool mockRet)
{
    g_mockGetTokenTypeFlag = mockRet;
}
}
}  // namespace OHOS
