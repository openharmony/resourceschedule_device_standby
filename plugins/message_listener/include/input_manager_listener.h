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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_PLUGINS_INCLUDE_INPUT_MANAGER_LISTENER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_PLUGINS_INCLUDE_INPUT_MANAGER_LISTENER_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include "event_handler.h"
#include "imessage_listener.h"

namespace OHOS {
namespace DevStandbyMgr {
class InputManagerListener : public IMesssageListener, public std::enable_shared_from_this<InputManagerListener> {
public:
    InputManagerListener();
    ~InputManagerListener() override = default;
    ErrCode StartListener() override;
    ErrCode StopListener() override;
    void OnCallbackEvent(int32_t switchValue);

private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {};
    int32_t subscriberId_ {0};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_PLUGINS_INCLUDE_INPUT_MANAGER_LISTENER_H