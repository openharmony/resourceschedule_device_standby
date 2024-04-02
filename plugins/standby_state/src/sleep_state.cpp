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

#include "sleep_state.h"

#include <cmath>
#include "time_service_client.h"

#include "standby_service_log.h"
#include "standby_config_manager.h"
#include "iconstraint_manager_adapter.h"
#include "istate_manager_adapter.h"
#include "time_provider.h"
#include "timed_task.h"

using namespace OHOS::MiscServices;
namespace OHOS {
namespace DevStandbyMgr {
namespace {
    constexpr int32_t DUMP_REPEAT_DETECTION_TIMEOUT = 100;
}

SleepState::SleepState(uint32_t curState, uint32_t curPhase, const std::shared_ptr<IStateManagerAdapter>&
    stateManager, std::shared_ptr<AppExecFwk::EventHandler>& handler): BaseState(curState,
    curPhase, stateManager, handler)
{
    maintInterval_ = StandbyConfigManager::GetInstance()->GetStandbyDurationList(SLEEP_MAINT_DURATOIN);
    nextState_ = StandbyState::MAINTENANCE;
}

ErrCode SleepState::Init(const std::shared_ptr<BaseState>& statePtr)
{
    auto callbackTask = [statePtr]() { statePtr->StartTransitNextState(statePtr); };
    enterStandbyTimerId_ = TimedTask::CreateTimer(false, 0, true, true, callbackTask);
    if (enterStandbyTimerId_ == 0) {
        STANDBYSERVICE_LOGE("%{public}s state init failed", STATE_NAME_LIST[GetCurState()].c_str());
        return ERR_STANDBY_STATE_INIT_FAILED;
    }

    if (!StandbyConfigManager::GetInstance()->GetStandbySwitch(DETECT_MOTION_CONFIG)) {
        return ERR_OK;
    }
    auto callback = [sleepState = shared_from_this()]() { sleepState->StartPeriodlyMotionDetection(); };
    repeatedDetectionTimerId_ = TimedTask::CreateTimer(true, REPEATED_MOTION_DETECTION_INTERVAL, true, false, callback);
    if (repeatedDetectionTimerId_ == 0) {
        STANDBYSERVICE_LOGE("%{public}s init failed", STATE_NAME_LIST[GetCurState()].c_str());
        return ERR_STANDBY_STATE_INIT_FAILED;
    }
    SetTimedTask(REPEATED_MOTION_DETECTION_TASK, repeatedDetectionTimerId_);
    return ERR_OK;
}

void SleepState::StartPeriodlyMotionDetection()
{
    handler_->PostTask([sleepState = shared_from_this()]() {
        sleepState->isRepeatedDetection_ = true;
        ConstraintEvalParam params{sleepState->curState_, sleepState->curPhase_,
            sleepState->curState_, sleepState->curPhase_};
        params.isRepeatedDetection_ = true;
        auto stateManagerPtr = sleepState->stateManager_.lock();
        if (!stateManagerPtr) {
            return;
        }
        stateManagerPtr->StartEvalCurrentState(params);
        }, REPEATED_MOTION_DETECTION_TASK);
}

ErrCode SleepState::UnInit()
{
    BaseState::UnInit();
    isRepeatedDetection_ = false;
    repeatedDetectionTimerId_ = 0;
    return ERR_OK;
}

ErrCode SleepState::BeginState()
{
    auto stateManagerPtr = stateManager_.lock();
    if (!stateManagerPtr) {
        STANDBYSERVICE_LOGE("state manager adapter is nullptr");
        return ERR_STATE_MANAGER_IS_NULLPTR;
    }
    isRepeatedDetection_ = false;
    int64_t maintIntervalTimeOut = 0;
    if (stateManagerPtr->GetPreState() == StandbyState::MAINTENANCE) {
        maintIntervalTimeOut = CalculateMaintTimeOut(stateManagerPtr, false);
        if (maintIntervalTimeOut != 0) {
            STANDBYSERVICE_LOGI("from maintenance to sleep, maintIntervalTimeOut is " SPUBI64,
                maintIntervalTimeOut);
            StartStateTransitionTimer(maintIntervalTimeOut);
        }
        return ERR_OK;
    }

    maintIntervalIndex_ = 0;
    curPhase_ = SleepStatePhase::SYS_RES_DEEP;
    maintIntervalTimeOut = CalculateMaintTimeOut(stateManagerPtr, true);
    STANDBYSERVICE_LOGI("maintIntervalTimeOut is " SPUBI64 " ms", maintIntervalTimeOut);

    handler_->PostTask([sleepState = shared_from_this()]() {
        BaseState::AcquireStandbyRunningLock();
        sleepState->TransitToPhase(sleepState->curPhase_, sleepState->curPhase_ + 1);
        }, TRANSIT_NEXT_PHASE_INSTANT_TASK);
    StartStateTransitionTimer(maintIntervalTimeOut);
    CheckScrenOffHalfHour();
    return ERR_OK;
}

void SleepState::TryToEnterNextPhase(const std::shared_ptr<IStateManagerAdapter>& stateManagerPtr,
    int32_t retryTimeOut)
{
    if (stateManagerPtr->IsEvalution()) {
        STANDBYSERVICE_LOGW("state is in evalution, postpone to enter next phase");
        handler_->PostTask([sleepState = shared_from_this(), stateManagerPtr, retryTimeOut]() {
            sleepState->TryToEnterNextPhase(stateManagerPtr, retryTimeOut);
            }, TRANSIT_NEXT_PHASE_INSTANT_TASK, retryTimeOut);
    } else if (curPhase_ < SleepStatePhase::END) {
        TransitToPhase(curPhase_, curPhase_ + 1);
    }
}

ErrCode SleepState::EndState()
{
    StopTimedTask(TRANSIT_NEXT_STATE_TIMED_TASK);
    StopTimedTask(REPEATED_MOTION_DETECTION_TASK);
    handler_->RemoveTask(TRANSIT_NEXT_STATE_TIMED_TASK);
    handler_->RemoveTask(TRANSIT_NEXT_PHASE_INSTANT_TASK);
    handler_->RemoveTask(REPEATED_MOTION_DETECTION_TASK);
    return ERR_OK;
}

bool SleepState::CheckTransitionValid(uint32_t nextState)
{
    if (nextState == StandbyState::NAP) {
        STANDBYSERVICE_LOGE("can not transit from sleep to nap");
        return false;
    }
    return true;
}

void SleepState::EndEvalCurrentState(bool evalResult)
{
    auto stateManagerPtr = stateManager_.lock();
    if (!stateManagerPtr) {
        STANDBYSERVICE_LOGW("state manager is nullptr, cannot end eval sleep state");
        return;
    }
    if (curPhase_ < SleepStatePhase::END) {
        if (evalResult) {
            TransitToPhaseInner(curPhase_, curPhase_ + 1);
        }
        SetPhaseTransitOrRepeatedTask();
        return;
    }
    if (!evalResult && isRepeatedDetection_) {
        stateManagerPtr->TransitToState(StandbyState::WORKING);
    }
    isRepeatedDetection_ = false;
}

void SleepState::SetPhaseTransitOrRepeatedTask()
{
    curPhase_ += 1;
    if (curPhase_ < SleepStatePhase::END) {
        handler_->PostTask([sleepState = shared_from_this()]() {
            sleepState->TransitToPhase(sleepState->curPhase_, sleepState->curPhase_ + 1);
            }, TRANSIT_NEXT_PHASE_INSTANT_TASK);
    } else {
        BaseState::ReleaseStandbyRunningLock();
        if (repeatedDetectionTimerId_ == 0 || !MiscServices::TimeServiceClient::GetInstance()->
            StartTimer(repeatedDetectionTimerId_, MiscServices::TimeServiceClient::GetInstance()->
            GetWallTimeMs() + REPEATED_MOTION_DETECTION_INTERVAL)) {
            STANDBYSERVICE_LOGE("sleep state set periodly task failed");
        }
    }
}

void SleepState::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
    if (argsInStr[DUMP_FIRST_PARAM] == DUMP_SIMULATE_SENSOR) {
        if (argsInStr[DUMP_SECOND_PARAM] == "--repeat") {
            StartPeriodlyMotionDetection();
            handler_->PostTask([sleepState = shared_from_this()]() {
                STANDBYSERVICE_LOGD("after 100ms, stop sensor");
                sleepState->stateManager_.lock()->EndEvalCurrentState(false);
                }, DUMP_REPEAT_DETECTION_TIMEOUT);
            result += "finished start repeated sensor\n";
        }
    }
}

void SleepState::CheckScrenOffHalfHour()
{
    auto stateManagerPtr = stateManager_.lock();
    if (!stateManagerPtr) {
        STANDBYSERVICE_LOGW("state manager is nullptr, cannot begin screen off half hour");
        return;
    }
    if (MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs() -
        stateManagerPtr->GetScreenOffTimeStamp() >= HALF_HOUR) {
        stateManagerPtr->OnScreenOffHalfHour(true, false);
    }
}

bool SleepState::IsInFinalPhase()
{
    return curPhase_ == SleepStatePhase::END;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS