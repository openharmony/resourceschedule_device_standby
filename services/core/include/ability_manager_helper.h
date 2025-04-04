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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_ABILITY_MGR_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_ABILITY_MGR_HELPER_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include <list>
#include "isystem_process_status_change.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace DevStandbyMgr {
class AbilityManagerHelper {
DECLARE_DELAYED_SINGLETON(AbilityManagerHelper);
public:
    static std::shared_ptr<AbilityManagerHelper> GetInstance();
    bool GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos);
private:
    AbilityManagerHelper(const AbilityManagerHelper&) = delete;
    AbilityManagerHelper& operator= (const AbilityManagerHelper&) = delete;
    AbilityManagerHelper(AbilityManagerHelper&&) = delete;
    AbilityManagerHelper& operator= (AbilityManagerHelper&&) = delete;
    std::mutex mutex_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_ABILITY_MGR_HELPER_H