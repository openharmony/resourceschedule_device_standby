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

#include "system_ability.h"
#include "iservice_registry.h"
#include "mock_sa_service.h"
#include "ipc_skeleton.h"

namespace {
    bool g_mockGetSystemAbilityManager = true;
    bool g_mockSendRequest = true;
    const int32_t TEST_RETVAL_ONREMOTEREQUEST = 10000;
}

namespace OHOS {
sptr<IRemoteObject> SystemAbility::GetSystemAbility(int32_t systemAbilityId)
{
    return nullptr;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    if (!g_mockGetSystemAbilityManager) {
        return nullptr;
    }
    sptr<IRemoteObject> registryObject = IPCSkeleton::GetContextObject();
    return iface_cast<ISystemAbilityManager>(registryObject);
}

int IPCObjectStub::SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(TEST_RETVAL_ONREMOTEREQUEST);
    if (g_mockSendRequest) {
        return NO_ERROR;
    }
    return ERR_DEAD_OBJECT;
}

namespace DevStandbyMgr {
void MockSaService::MockGetSystemAbilityManager(bool mockRet)
{
    g_mockGetSystemAbilityManager = mockRet;
}

void MockSaService::MockSendRequest(bool mockRet)
{
    g_mockSendRequest = mockRet;
}
}
}  // namespace OHOS