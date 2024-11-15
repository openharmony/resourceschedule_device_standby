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

#include "ability_manager_helper.h"
#include "iservice_registry.h"

#include "standby_service_log.h"
#include "standby_service_errors.h"

namespace OHOS {
namespace DevStandbyMgr {
AbilityManagerHelper::AbilityManagerHelper()
{
}

AbilityManagerHelper::~AbilityManagerHelper()
{
}

std::shared_ptr<AbilityManagerHelper> AbilityManagerHelper::GetInstance()
{
    return DelayedSingleton<AbilityManagerHelper>::GetInstance();
}

bool WEAK_FUNC AbilityManagerHelper::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto ret = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager()->
        GetRunningSystemProcess(systemProcessInfos);
    if (ret != ERR_OK)  {
        STANDBYSERVICE_LOGE("failed to GetRunningSystemProcess");
        return false;
    }
    return true;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS