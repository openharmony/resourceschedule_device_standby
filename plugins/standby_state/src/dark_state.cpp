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

#include "dark_state.h"

#include "time_provider.h"
#include "timed_task.h"

#include "standby_service_log.h"
#include "standby_config_manager.h"
#include "iconstraint_manager_adapter.h"
#include "istate_manager_adapter.h"


namespace OHOS {
namespace DevStandbyMgr {
DarkState::DarkState(uint32_t curState, uint32_t curPhase, const std::shared_ptr<IStateManagerAdapter>&
    stateManager, std::shared_ptr<AppExecFwk::EventHandler>& handler): BaseState(curState, curPhase,
    stateManager, handler)
{
    darkTimeOut_ = TimeConstant::MSEC_PER_SEC * StandbyConfigManager::GetInstance()->GetStandbyParam(DARK_TIMEOUT);
    nextState_ = StandbyState::DARK;
    if (StandbyConfigManager::GetInstance()->GetStandbySwitch(NAP_SWITCH)) {
        nextState_ = StandbyState::NAP;
    } else if (StandbyConfigManager::GetInstance()->GetStandbySwitch(SLEEP_SWITCH)) {
        nextState_ = StandbyState::SLEEP;
    }
}

ErrCode DarkState::BeginState()
{
    curPhase_ = 0;
    if (nextState_ != StandbyState::DARK) {
        return StartStateTransitionTimer(darkTimeOut_);
    }
    return ERR_OK;
}

ErrCode DarkState::EndState()
{
    StopTimedTask(TRANSIT_NEXT_STATE_TIMED_TASK);
    handler_->RemoveTask(TRANSIT_NEXT_STATE_TIMED_TASK);
    return ERR_OK;
}

bool DarkState::CheckTransitionValid(uint32_t nextState)
{
    if (nextState == StandbyState::MAINTENANCE) {
        STANDBYSERVICE_LOGE("can not transit from dark to maintenance");
        return false;
    }
    return true;
}

void DarkState::EndEvalCurrentState(bool evalResult)
{
    auto stateManagerPtr = stateManager_.lock();
    if (!stateManagerPtr) {
        STANDBYSERVICE_LOGW("state manager is nullptr, can not end evalution current state");
        return;
    }
    if (!evalResult) {
        STANDBYSERVICE_LOGD("constraint evalution failed, block current state");
        stateManagerPtr->BlockCurrentState();
    } else if (nextState_ != StandbyState::DARK) {
        stateManagerPtr->TransitToStateInner(nextState_);
    }
}
}  // namespace DevStandbyMgr
}  // namespace OHOS