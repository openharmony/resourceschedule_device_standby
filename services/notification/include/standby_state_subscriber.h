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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_NOTIFICATION_INCLUDE_NOTIFICATION_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_NOTIFICATION_INCLUDE_NOTIFICATION_HELPER_H

#include <list>
#include <map>
#include <memory>
#include <algorithm>

#include "singleton.h"
#include "iremote_object.h"

#include "allow_type.h"
#include "single_instance.h"
#include "standby_service_errors.h"
#include "istandby_service_subscriber.h"
#include "standby_state.h"
#include "istate_manager_adapter.h"

namespace OHOS {
namespace DevStandbyMgr {
class SubscriberDeathRecipient;
class StandbyStateSubscriber : public std::enable_shared_from_this<StandbyStateSubscriber> {
DECLARE_SINGLE_INSTANCE(StandbyStateSubscriber);

public:
    ErrCode AddSubscriber(const sptr<IStandbyServiceSubscriber>& subscriber);
    ErrCode RemoveSubscriber(const sptr<IStandbyServiceSubscriber>& subscriber);
    void ReportStandbyState(uint32_t curState);
    void ReportAllowListChanged(int32_t uid, const std::string& name, uint32_t allowType, bool added);
    void HandleSubscriberDeath(const wptr<IRemoteObject>& remote);
    void ShellDump(const std::vector<std::string>& argsInStr, std::string& result);
    void NotifyAllowChangedByCommonEvent(int32_t uid, const std::string& name, uint32_t allowType, bool added);

private:
    void NotifyIdleModeByCallback(bool napped, bool sleeping);
    void NotifyIdleModeByCommonEvent(bool napped, bool sleeping);
    void NotifyAllowChangedByCallback(int32_t uid, const std::string& name, uint32_t allowType, bool added);
    std::list<sptr<IStandbyServiceSubscriber>>::iterator FindSubcriberObject(sptr<IRemoteObject>& proxy);

private:
    std::mutex subscriberLock_ {};
    std::list<sptr<IStandbyServiceSubscriber>> subscriberList_ {};
    sptr<SubscriberDeathRecipient> deathRecipient_ {nullptr};
    std::shared_ptr<IStateManagerAdapter> standbyStateManager_ {nullptr};
};

class SubscriberDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    DISALLOW_COPY_AND_MOVE(SubscriberDeathRecipient);
    explicit SubscriberDeathRecipient();
    ~SubscriberDeathRecipient() override;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_NOTIFICATION_INCLUDE_NOTIFICATION_HELPER_H