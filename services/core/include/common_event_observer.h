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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_DEV_COMMON_EVENT_OBSERVER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_DEV_COMMON_EVENT_OBSERVER_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include "common_event_subscriber.h"
#include "common_event_subscribe_info.h"
#include "event_handler.h"

namespace OHOS {
namespace DevStandbyMgr {
class CommonEventObserver : public EventFwk::CommonEventSubscriber,
    public std::enable_shared_from_this<CommonEventObserver> {
public:
    explicit CommonEventObserver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        const std::shared_ptr<AppExecFwk::EventHandler>& handler);
    ~CommonEventObserver() override = default;
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
    bool Subscribe();
    bool Unsubscribe();
    static std::shared_ptr<CommonEventObserver> CreateCommonEventObserver(
        const std::shared_ptr<AppExecFwk::EventHandler>& handler);

private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_DEV_COMMON_EVENT_OBSERVER_H