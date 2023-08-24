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
#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STATE_MANAGER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STATE_MANAGER_H

#include "istate_manager_adapter.h"

namespace OHOS {
namespace DevStandbyMgr {
class StateManagerAdapter : public IStateManagerAdapter {
public:
    StateManagerAdapter() = default;
    ~StateManagerAdapter() override = default;
    bool Init() override;
    bool UnInit() override;
    void HandleEvent(const StandbyMessage& message) override;
    uint32_t GetCurState() override;
    uint32_t GetPreState() override;

    ErrCode StartEvalCurrentState(const ConstraintEvalParam& params) override;
    ErrCode EndEvalCurrentState(bool evalResult) override;

    void BlockCurrentState() override;
    void UnblockCurrentState() override;
    void OnScreenOffHalfHour(bool scrOffHalfHourCtrl, bool repeated) override;

    ErrCode TransitToState(uint32_t nextState) override;
    ErrCode TransitToStateInner(uint32_t nextState) override;
    void StopEvalution() override;
    void HandleOpenCloseLid(const StandbyMessage& message);
    void ShellDump(const std::vector<std::string>& argsInStr, std::string& result) override;
private:
    void SendNotification(uint32_t preState, bool needDispatchEvent);
    bool CheckTransitionValid(uint32_t curState, uint32_t nextState);
    ErrCode ExitStandby(uint32_t nextState);
    ErrCode EnterStandby(uint32_t nextState);
    ErrCode TransitWithMaint(uint32_t nextState);
    void OnScreenOffHalfHourInner(bool scrOffHalfHourCtrl, bool repeated);

    void HandleCommonEvent(const StandbyMessage& message);
    void HandleScreenStatus(const StandbyMessage& message);
    void HandleScrOffHalfHour(const StandbyMessage& message);
    bool CheckEnterDarkState(const StandbyMessage& message);
    void DumpShowDetailInfo(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpEnterSpecifiedState(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpActivateMotion(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpResetState(const std::vector<std::string>& argsInStr, std::string& result);
    void RecordStateTransition();
private:
    std::shared_ptr<BaseState> darkStatePtr_ {nullptr};
    std::shared_ptr<BaseState> maintStatePtr_ {nullptr};
    std::shared_ptr<BaseState> napStatePtr_ {nullptr};
    std::shared_ptr<BaseState> sleepStatePtr_ {nullptr};
    std::shared_ptr<BaseState> workingStatePtr_ {nullptr};
    std::vector<std::shared_ptr<BaseState>> indexToState_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS

#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STATE_MANAGER_H