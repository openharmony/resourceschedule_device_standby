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

#include "charge_state_monitor.h"
#ifdef STANDBY_BATTERY_MANAGER_ENABLE
#include "battery_srv_client.h"
#endif
#include "standby_service_impl.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
bool ChargeStateMonitor::Init()
{
    auto &constraintManager = StandbyServiceImpl::GetInstance()->GetConstraintManager();
    ConstraintEvalParam params{StandbyState::WORKING, 0, StandbyState::DARK, 0};
    constraintManager->RegisterConstraintCallback(params, shared_from_this());
    return true;
}

void ChargeStateMonitor::StartMonitoring()
{
    bool res {true};
    #ifdef STANDBY_BATTERY_MANAGER_ENABLE
    auto chargingStatus = PowerMgr::BatterySrvClient::GetInstance().GetChargingStatus();
    if (chargingStatus == PowerMgr::BatteryChargeState::CHARGE_STATE_ENABLE || chargingStatus ==
        PowerMgr::BatteryChargeState::CHARGE_STATE_FULL) {
        STANDBYSERVICE_LOGI("can not enter next state due to the charging status");
        res = false;
    }
    #endif
    StandbyServiceImpl::GetInstance()->GetStateManager()->EndEvalCurrentState(res);
}

void ChargeStateMonitor::StopMonitoring()
{}
} // DevStandbyMgr
} // OHOS
