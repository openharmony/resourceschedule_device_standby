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

#include "base_state.h"

#include <functional>
#include "time_service_client.h"

#include "standby_messsage.h"
#include "standby_service_log.h"

#include "istate_manager_adapter.h"
#include "timed_task.h"
#include "time_provider.h"
#include "standby_service_impl.h"

using namespace OHOS::MiscServices;
namespace OHOS {
namespace DevStandbyMgr {

std::shared_ptr<PowerMgr::RunningLock> BaseState::standbyRunningLock_ = nullptr;
bool BaseState::runningLockStatus_ = false;

ErrCode BaseState::Init()
{
    auto callbackTask = [this]() { this->StartTransitNextState(); };
    enterStandbyTimerId_ = TimedTask::CreateTimer(false, 0, true, callbackTask);
    if (enterStandbyTimerId_ == 0) {
        STANDBYSERVICE_LOGE("%{public}s state init failed", STATE_NAME_LIST[GetCurState()].c_str());
        return ERR_STANDBY_STATE_INIT_FAILED;
    }
    return ERR_OK;
}

ErrCode BaseState::UnInit()
{
    DestroyAllTimedTask();
    enterStandbyTimerId_ = 0;
    return ERR_OK;
}

uint32_t BaseState::GetCurState()
{
    return curState_;
}

uint32_t BaseState::GetCurInnerPhase()
{
    return curPhase_;
}

void BaseState::StartTransitNextState()
{
    handler_->PostTask([this]() {
        STANDBYSERVICE_LOGD("due to timeout, try to enter %{public}s state from %{public}s",
            STATE_NAME_LIST[nextState_].c_str(), STATE_NAME_LIST[curState_].c_str());
        BaseState::AcquireStandbyRunningLock();
        auto stateManagerPtr = stateManager_.lock();
        if (!stateManagerPtr) {
            STANDBYSERVICE_LOGW("state manager is nullptr, can not transit to next state");
            BaseState::ReleaseStandbyRunningLock();
            return;
        }
        if (stateManagerPtr->IsEvalution()) {
            STANDBYSERVICE_LOGW("state is in evalution, stop evalution and enter next state");
            stateManagerPtr->StopEvalution();
        }
        if (stateManagerPtr->TransitToState(nextState_) != ERR_OK) {
            STANDBYSERVICE_LOGW("can not transit to state %{public}d, block current state", nextState_);
            stateManagerPtr->BlockCurrentState();
            BaseState::ReleaseStandbyRunningLock();
        }
        }, TRANSIT_NEXT_STATE_TIMED_TASK);
}

void BaseState::TransitToPhase(uint32_t curPhase, uint32_t nextPhase)
{
    ConstraintEvalParam params{curState_, curPhase, curState_, nextPhase};
    stateManager_.lock()->StartEvalCurrentState(params);
}

void BaseState::TransitToPhaseInner(uint32_t prePhase, uint32_t curPhase)
{
    auto stateManagerPtr = stateManager_.lock();
    if (!stateManagerPtr) {
        STANDBYSERVICE_LOGW("state manager is nullptr, can not implement function to enter next phase");
        return;
    }
    StandbyMessage message(StandbyMessageType::PHASE_TRANSIT);
    message.want_ = AAFwk::Want{};
    message.want_->SetParam(CURRENT_STATE, static_cast<int32_t>(curState_));
    message.want_->SetParam(PREVIOUS_PHASE, static_cast<int32_t>(prePhase));
    message.want_->SetParam(CURRENT_PHASE, static_cast<int32_t>(curPhase));
    StandbyServiceImpl::GetInstance()->DispatchEvent(message);
    STANDBYSERVICE_LOGI("phase transit succeed, phase form %{public}d to %{public}d",
        static_cast<int32_t>(prePhase), static_cast<int32_t>(curPhase));
}

bool BaseState::IsInFinalPhase()
{
    return true;
}

void BaseState::OnStateBlocked()
{}

void BaseState::SetTimedTask(const std::string& timedTaskName, uint64_t timedTaskId)
{
    if (auto iter = timedTaskMap_.find(timedTaskName); iter == timedTaskMap_.end()) {
        timedTaskMap_.emplace(timedTaskName, timedTaskId);
    } else {
        iter->second = timedTaskId;
    }
}

ErrCode BaseState::StartStateTransitionTimer(int64_t triggerTime)
{
    if (enterStandbyTimerId_ == 0 || !MiscServices::TimeServiceClient::GetInstance()->
        StartTimer(enterStandbyTimerId_, MiscServices::TimeServiceClient::GetInstance()->
        GetWallTimeMs() + triggerTime)) {
        STANDBYSERVICE_LOGE("%{public}s state set timed task failed", STATE_NAME_LIST[nextState_].c_str());
        return ERR_STANDBY_TIMER_SERVICE_ERROR;
    }
    SetTimedTask(TRANSIT_NEXT_STATE_TIMED_TASK, enterStandbyTimerId_);
    return ERR_OK;
}

ErrCode BaseState::StopTimedTask(const std::string& timedTaskName)
{
    if (auto iter = timedTaskMap_.find(timedTaskName); iter == timedTaskMap_.end()) {
        STANDBYSERVICE_LOGW("timedTask %{public}s not exist", timedTaskName.c_str());
        return ERR_STANDBY_TIMERID_NOT_EXIST;
    } else if (iter->second > 0) {
        MiscServices::TimeServiceClient::GetInstance()->StopTimer(iter->second);
    }

    return ERR_OK;
}

void BaseState::DestroyAllTimedTask()
{
    for (auto& [timeTaskName, timerId] : timedTaskMap_) {
        handler_->RemoveTask(timeTaskName);
        if (timerId > 0) {
            TimeServiceClient::GetInstance()->StopTimer(timerId);
            TimeServiceClient::GetInstance()->DestroyTimer(timerId);
        }
    }
    timedTaskMap_.clear();
}

void BaseState::InitRunningLock()
{
    runningLockStatus_ = false;
    standbyRunningLock_ = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("StandbyRunningLock",
        PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND);
}

void BaseState::AcquireStandbyRunningLock()
{
    if (runningLockStatus_) {
        return;
    }
    standbyRunningLock_->Lock();
    runningLockStatus_ = true;
    STANDBYSERVICE_LOGD("acquire standby running lock, status is %{public}d", runningLockStatus_);
}

void BaseState::ReleaseStandbyRunningLock()
{
    if (!runningLockStatus_) {
        return;
    }
    standbyRunningLock_->UnLock();
    runningLockStatus_ = false;
    STANDBYSERVICE_LOGD("release standby running lock, status is %{public}d", runningLockStatus_);
}

void BaseState::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
    return;
}

int64_t StateWithMaint::CalculateMaintTimeOut(const std::shared_ptr<IStateManagerAdapter>& stateManagerPtr,
    bool isFirstInterval)
{
    int64_t maintIntervalTimeOut {0};
    if (maintInterval_.empty()) {
        STANDBYSERVICE_LOGE("maintenance interval config error, can not enter maintence state");
        return 0;
    }
    if (isFirstInterval) {
        maintIntervalTimeOut = TimeConstant::MSEC_PER_SEC * maintInterval_[maintIntervalIndex_];
    } else {
        maintIntervalIndex_ = std::min(maintIntervalIndex_ + 1, static_cast<int32_t>(maintInterval_.size() - 1));
        maintIntervalTimeOut = TimeConstant::MSEC_PER_SEC * maintInterval_[maintIntervalIndex_];
    }
    return maintIntervalTimeOut;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS