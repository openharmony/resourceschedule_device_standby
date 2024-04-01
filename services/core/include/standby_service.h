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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STANDBY_SERVICE_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STANDBY_SERVICE_H

#include <ctime>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "singleton.h"
#include "system_ability.h"

#include "standby_service_stub.h"
#include "single_instance.h"

namespace OHOS {
namespace DevStandbyMgr {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

class StandbyService final : public SystemAbility, public StandbyServiceStub {
    DECLARE_SYSTEM_ABILITY(StandbyService);
    DECLARE_SINGLE_INSTANCE(StandbyService);
public:
    StandbyService(const int32_t systemAbilityId, bool runOnCreate);
    void OnStart() final;
    void OnStop() final;

    ErrCode SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber) override;
    ErrCode UnsubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber) override;
    ErrCode ApplyAllowResource(const sptr<ResourceRequest>& resourceRequest) override;
    ErrCode UnapplyAllowResource(const sptr<ResourceRequest>& resourceRequest) override;
    ErrCode GetAllowList(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
        uint32_t reasonCode) override;
    ErrCode IsDeviceInStandby(bool& isStandby) override;
    ErrCode ReportWorkSchedulerStatus(bool started, int32_t uid, const std::string& bundleName) override;
    ErrCode GetRestrictList(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
        uint32_t reasonCode) override;
    ErrCode IsStrategyEnabled(const std::string& strategyName, bool& isEnabled) override;
    ErrCode ReportDeviceStateChanged(DeviceStateType type, bool enabled) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void AddPluginSysAbilityListener(int32_t systemAbilityId);
    ErrCode NotifySystemAbilityStatusChanged(bool isAdded, int32_t systemAbilityId);
    ErrCode HandleEvent(const uint32_t resType, const int64_t value, const std::string &sceneInfo) override;

private:
    void DumpUsage(std::string& result);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    bool CheckProcessNamePermission(const std::string& processName);

private:
    std::mutex systemAbilityLock_ {};
    ServiceRunningState state_ {ServiceRunningState::STATE_NOT_START};
    uint32_t dependsReady_ = 0;
    std::mutex listenedSALock_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STANDBY_SERVICE_H