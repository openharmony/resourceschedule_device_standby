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

#include "maintenance_state.h"

#include "standby_service_log.h"
#include "standby_config_manager.h"
#include "iconstraint_manager_adapter.h"
#include "istate_manager_adapter.h"
#include "time_provider.h"

namespace OHOS {
namespace DevStandbyMgr {
ErrCode MaintenanceState::BeginState()
{
    auto stateManagerPtr = stateManager_.lock();
    if (!stateManagerPtr) {
        STANDBYSERVICE_LOGW("state manager is nullptr, can not begin current state");
        return ERR_STATE_MANAGER_IS_NULLPTR;
    }
    nextState_ = stateManagerPtr->GetPreState();
    int64_t endMaintTimeOut = 0;
    if (nextState_ == StandbyState::NAP) {
        endMaintTimeOut = StandbyConfigManager::GetInstance()->GetStandbyParam(NAP_MAINTENANCE_TIMEOUT);
    } else {
        endMaintTimeOut = StandbyConfigManager::GetInstance()->GetStandbyParam(SLEEP_MAINT_TIMEOUT);
    }
    endMaintTimeOut = endMaintTimeOut * TimeConstant::MSEC_PER_SEC;
    return StartStateTransitionTimer(endMaintTimeOut);
}

ErrCode MaintenanceState::EndState()
{
    StopTimedTask(TRANSIT_NEXT_STATE_TIMED_TASK);
    handler_->RemoveTask(TRANSIT_NEXT_STATE_TIMED_TASK);
    return ERR_OK;
}

bool MaintenanceState::CheckTransitionValid(uint32_t nextState)
{
    if (nextState == StandbyState::DARK) {
        STANDBYSERVICE_LOGE("can not transit from maintenance to dark");
        return false;
    }
    return true;
}

void MaintenanceState::EndEvalCurrentState(bool evalResult)
{}
}  // namespace DevStandbyMgr
}  // namespace OHOS