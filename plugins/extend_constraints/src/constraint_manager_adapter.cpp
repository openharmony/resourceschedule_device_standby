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

#include "constraint_manager_adapter.h"
#include "standby_service_impl.h"
#include "standby_service_log.h"
#include "standby_config_manager.h"
#ifdef STANDBY_SENSORS_SENSOR_ENABLE
#include "motion_sensor_monitor.h"
#endif
#include "charge_state_monitor.h"
#include "base_state.h"

namespace OHOS {
namespace DevStandbyMgr {
bool ConstraintManagerAdapter::Init()
{
    stateManager_ = StandbyServiceImpl::GetInstance()->GetStateManager();
    if (stateManager_.expired()) {
        STANDBYSERVICE_LOGI("constraint manager plugin initialization failed");
        return false;
    }
    constraintMonitorList_.emplace_back(std::make_shared<ChargeStateMonitor>());
    if (StandbyConfigManager::GetInstance()->GetStandbySwitch(DETECT_MOTION_CONFIG)) {
        #ifdef STANDBY_SENSORS_SENSOR_ENABLE
        ConstraintEvalParam motionDetectParams{StandbyState::NAP, NapStatePhase::END, StandbyState::SLEEP,
            SleepStatePhase::SYS_RES_DEEP};
        motionConstraint_ = std::make_shared<MotionSensorMonitor>(MOTION_DETECTION_TIMEOUT, REST_TIMEOUT,
            TOTAL_TIMEOUT, motionDetectParams);
        constraintMonitorList_.emplace_back(motionConstraint_);

        ConstraintEvalParam repeatedMotionParams{StandbyState::SLEEP, SleepStatePhase::END, StandbyState::SLEEP,
            SleepStatePhase::END};
        repeatedMotionParams.isRepeatedDetection_ = true;
        repeatedMotionConstraint_ = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
        constraintMonitorList_.emplace_back(repeatedMotionConstraint_);
        #endif
    }
    for (const auto& constraintMonitor : constraintMonitorList_) {
        constraintMonitor->Init();
    }
    STANDBYSERVICE_LOGI("constraint manager plugin initialization succeed");
    return true;
}

bool ConstraintManagerAdapter::UnInit()
{
    constraintMonitorList_.clear();
    constraintMap_.clear();
    stateManager_.reset();
    curMonitor_.reset();
    isEvaluation_ = false;
    repeatedMotionConstraint_.reset();
    motionConstraint_.reset();
    return true;
}

ErrCode ConstraintManagerAdapter::StartEvalution(const ConstraintEvalParam& params)
{
    if (isEvaluation_) {
        STANDBYSERVICE_LOGE("can not start evalution repeatedly");
        return ERR_STANDBY_STATE_TIMING_SEQ_ERROR;
    }
    STANDBYSERVICE_LOGD("start constraint evalution");
    isEvaluation_ = true;
    uint32_t evalutionHash = params.GetHashValue();
    auto iter = constraintMap_.find(evalutionHash);
    if (iter == constraintMap_.end()) {
        STANDBYSERVICE_LOGD("constraint evalution is nullptr, pass");
        curMonitor_ = nullptr;
        auto stateManagerPtr = stateManager_.lock();
        if (!stateManagerPtr) {
            STANDBYSERVICE_LOGW("state manager is nullptr, can not end evalution");
            return ERR_STATE_MANAGER_IS_NULLPTR;
        } else {
            stateManagerPtr->EndEvalCurrentState(true);
        }
    } else {
        curMonitor_ = iter->second;
        if (curMonitor_) {
            curMonitor_->StartMonitoring();
        }
    }
    return ERR_OK;
}

ErrCode ConstraintManagerAdapter::StopEvalution()
{
    if (!isEvaluation_)  {
        STANDBYSERVICE_LOGE("can not stop unused evalution");
        return ERR_STANDBY_STATE_TIMING_SEQ_ERROR;
    }
    isEvaluation_ = false;
    if (!curMonitor_) {
        return ERR_OK;
    }
    curMonitor_->StopMonitoring();
    curMonitor_ = nullptr;
    return ERR_OK;
}

void ConstraintManagerAdapter::RegisterConstraintCallback(const ConstraintEvalParam& params,
    const std::shared_ptr<IConstraintMonitor>& monitor)
{
    constraintMap_.emplace(params.GetHashValue(), monitor);
}

void ConstraintManagerAdapter::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
