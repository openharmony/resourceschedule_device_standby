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

#include "input_manager_listener.h"

#include "input_manager.h"
#include "want.h"

#include "standby_service_impl.h"
#include "istate_manager_adapter.h"

namespace OHOS {
namespace DevStandbyMgr {
InputManagerListener::InputManagerListener()
{
    handler_ = StandbyServiceImpl::GetInstance()->GetHandler();
}

ErrCode WEAK_FUNC InputManagerListener::StartListener()
{
    STANDBYSERVICE_LOGD("start input manager listener");
    subscriberId_ = MMI::InputManager::GetInstance()->SubscribeSwitchEvent(
        [listener = shared_from_this()] (std::shared_ptr<OHOS::MMI::SwitchEvent> switchEvent) {
            listener->OnCallbackEvent(switchEvent->GetSwitchValue());
            });
    STANDBYSERVICE_LOGD("start input manager listner, result is %{public}d", subscriberId_);
    return ERR_OK;
}

ErrCode WEAK_FUNC InputManagerListener::StopListener()
{
    if (subscriberId_ < 0) {
        return ERR_STANDBY_STOP_LISENER_FAILED;
    }
    MMI::InputManager::GetInstance()->UnsubscribeSwitchEvent(subscriberId_);
    subscriberId_ = 0;
    return ERR_OK;
}

void InputManagerListener::OnCallbackEvent(int32_t switchValue)
{
    StandbyMessage msg {StandbyMessageType::COMMON_EVENT};
    if (switchValue == MMI::SwitchEvent::SWITCH_OFF) {
        STANDBYSERVICE_LOGI("lid close event received");
        msg.action_ = LID_CLOSE;
    } else {
        STANDBYSERVICE_LOGI("lid open event received");
        msg.action_ = LID_OPEN;
    }
    handler_->PostTask([&msg]() {
        StandbyServiceImpl::GetInstance()->DispatchEvent(msg);
    });
}
}  // namespace DevStandbyMgr
}  // namespace OHOS