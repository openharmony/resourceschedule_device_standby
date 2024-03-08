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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_COMMON_EVENT_OBSERVER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_COMMON_EVENT_OBSERVER_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include <memory>
#include <unordered_map>

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_subscribe_info.h"
#include "event_handler.h"
#include "imessage_listener.h"
#include "istate_manager_adapter.h"
#include "standby_service_errors.h"

namespace OHOS {
namespace DevStandbyMgr {
class CommonEventListener : public EventFwk::CommonEventSubscriber, public IMesssageListener,
    public std::enable_shared_from_this<CommonEventListener> {
public:
    explicit CommonEventListener(const EventFwk::CommonEventSubscribeInfo& subscribeInfo);
    ~CommonEventListener() override = default;
    ErrCode StartListener() override;
    ErrCode StopListener() override;
    void OnReceiveEvent(const EventFwk::CommonEventData& eventData) override;
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler>& handler);
private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {nullptr};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_COMMON_EVENT_OBSERVER_H