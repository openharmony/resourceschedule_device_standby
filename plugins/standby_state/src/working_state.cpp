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

#include "working_state.h"

#include "standby_service_log.h"
#include "standby_config_manager.h"
#include "iconstraint_manager_adapter.h"
#include "istate_manager_adapter.h"

namespace OHOS {
namespace DevStandbyMgr {
ErrCode WorkingState::BeginState()
{
    curPhase_ = 0;
    handler_->PostTask([working = shared_from_this()]() {
        working->checkScreenStatus();
        }, TRANSIT_NEXT_STATE_CONDITION_TASK);
    return ERR_OK;
}

// check screen status, if screen off, traisit to dark status
void WorkingState::checkScreenStatus()
{
    auto stateManagerPtr = stateManager_.lock();
    if (!stateManagerPtr) {
        return;
    }
    if (!stateManagerPtr->IsScreenOn()) {
        stateManagerPtr->TransitToState(StandbyState::DARK);
    }
}

ErrCode WorkingState::Init(const std::shared_ptr<BaseState>& thisPtr)
{
    return ERR_OK;
}

ErrCode WorkingState::UnInit()
{
    return ERR_OK;
}

ErrCode WorkingState::EndState()
{
    if (!stateManager_.lock()) {
        STANDBYSERVICE_LOGE("state manager adapter is nullptr");
        return ERR_STATE_MANAGER_IS_NULLPTR;
    }
    handler_->RemoveTask(TRANSIT_NEXT_STATE_CONDITION_TASK);
    return ERR_OK;
}

bool WorkingState::CheckTransitionValid(uint32_t nextState)
{
    if (nextState == StandbyState::MAINTENANCE) {
        STANDBYSERVICE_LOGE("can not transit from working to maintenance");
        return false;
    }
    return true;
}

void WorkingState::EndEvalCurrentState(bool evalResult)
{
    auto stateManagerPtr = stateManager_.lock();
    if (!stateManagerPtr) {
        STANDBYSERVICE_LOGW("state manager is nullptr, cannot end eval working state");
        return;
    }
    if (evalResult) {
        stateManagerPtr->TransitToStateInner(StandbyState::DARK);
    } else {
        stateManagerPtr->BlockCurrentState();
    }
}
}  // namespace DevStandbyMgr
}  // namespace OHOS