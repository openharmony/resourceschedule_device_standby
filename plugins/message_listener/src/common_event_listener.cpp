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

#include "common_event_listener.h"

#include "bundle_constants.h"
#include "want.h"
#include "common_event_manager.h"
#include "common_event_support.h"

#include "standby_service_impl.h"
#include "istate_manager_adapter.h"
#include "istandby_service.h"
#include "call_manager_inner_type.h"
#include "wifi_p2p_msg.h"

namespace OHOS {
namespace DevStandbyMgr {
using namespace OHOS::Telephony;
using namespace OHOS::Wifi;
CommonEventListener::CommonEventListener(const EventFwk::CommonEventSubscribeInfo& subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
    handler_ = StandbyServiceImpl::GetInstance()->GetHandler();
    standbyStateManager_ = StandbyServiceImpl::GetInstance()->GetStateManager();
}

ErrCode WEAK_FUNC CommonEventListener::StartListener()
{
    return ERR_OK;
}

ErrCode WEAK_FUNC CommonEventListener::StopListener()
{
    return ERR_OK;
}

void WEAK_FUNC CommonEventListener::HandleCallStateChanged(int32_t state)
{
    bool disabled = (state == static_cast<int32_t>(TelCallState::CALL_STATUS_UNKNOWN) ||
        state == static_cast<int32_t>(TelCallState::CALL_STATUS_DISCONNECTED) ||
        state == static_cast<int32_t>(TelCallState::CALL_STATUS_IDLE));
    DeviceStateCache::GetInstance()->SetDeviceState(
        static_cast<int32_t>(DeviceStateType::TELEPHONE_STATE_CHANGE), !disabled);
}

void WEAK_FUNC CommonEventListener::HandleP2PStateChanged(int32_t state)
{
    bool disabled = (state == static_cast<int32_t>(P2pState::P2P_STATE_IDLE) ||
        state == static_cast<int32_t>(P2pState::P2P_STATE_NONE) ||
        state == static_cast<int32_t>(P2pState::P2P_STATE_CLOSED));
    DeviceStateCache::GetInstance()->SetDeviceState(
        static_cast<int32_t>(DeviceStateType::WIFI_P2P_CHANGE), !disabled);
}

void CommonEventListener::OnReceiveEvent(const EventFwk::CommonEventData& eventData)
{
    return;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS