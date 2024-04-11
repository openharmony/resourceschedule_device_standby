/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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


#include "listener_manager_adapter.h"

#include "common_event_support.h"

#include "standby_service_log.h"
#include "device_standby_switch.h"
#include "standby_service_impl.h"
#ifdef STANDBY_MULTIMODALINPUT_INPUT_ENABLE
#include "input_manager_listener.h"
#endif
#include "standby_service.h"

#include "standby_config_manager.h"
#include "system_ability_definition.h"
#ifdef ENABLE_BACKGROUND_TASK_MGR
#include "background_task_listener.h"
#endif

namespace OHOS {
namespace DevStandbyMgr {
bool ListenerManagerAdapter::Init()
{
    if (STANDBT_MODE != StandbyMode::PHONEMODE &&
            STANDBT_MODE != StandbyMode::TABLETMODE &&
            STANDBT_MODE != StandbyMode::UNKNOWN) {
        STANDBYSERVICE_LOGD("listener manager plugin initialization failed");
        return false;
    }
    #ifdef STANDBY_MULTIMODALINPUT_INPUT_ENABLE
    messageListenerList_.emplace_back(std::make_shared<InputManagerListener>());
    #endif
    #ifdef ENABLE_BACKGROUND_TASK_MGR
    // network and running lock strategy need background task listener
    STANDBYSERVICE_LOGI("add background task listener");
    std::shared_ptr<IMesssageListener> bgtaskListener_ = std::make_shared<BackgroundTaskListener>();
    listenerPluginMap_.emplace(BACKGROUND_TASK_MANAGER_SERVICE_ID, bgtaskListener_);
    #endif
    STANDBYSERVICE_LOGI("listener manager plugin initialization succeed");
    return true;
}

bool ListenerManagerAdapter::UnInit()
{
    StopListener();
    messageListenerList_.clear();
    return true;
}

void ListenerManagerAdapter::HandleEvent(const StandbyMessage& message)
{
    switch (message.eventId_) {
        case StandbyMessageType::SYS_ABILITY_STATUS_CHANGED:
            UpdateListenerList(message);
            break;
        default:
            break;
    }
}

void ListenerManagerAdapter::UpdateListenerList(const StandbyMessage& message)
{
    bool isAdded = message.want_->GetBoolParam(SA_STATUS, false);
    int32_t systemAbilityId = message.want_->GetIntParam(SA_ID, -1);
    if (isAdded) {
        // add listener if system ablity started
        AddSystemServiceListener(systemAbilityId);
        return;
    }
    RemoveSystemServiceListener(systemAbilityId);
}

// when system ability is added, add relative listener
void ListenerManagerAdapter::AddSystemServiceListener(int32_t systemAbilityId)
{
    auto iter = listenerPluginMap_.find(systemAbilityId);
    if (iter == listenerPluginMap_.end()) {
        return;
    }
    STANDBYSERVICE_LOGI("%{public}d added, start listener", systemAbilityId);
    std::shared_ptr<IMesssageListener> listener = iter->second;
    if (listener->StartListener() == ERR_OK) {
        messageListenerList_.emplace_back(listener);
    }
}

// when system ability is removed, remove relative listener
void ListenerManagerAdapter::RemoveSystemServiceListener(int32_t systemAbilityId)
{
    auto iter = listenerPluginMap_.find(systemAbilityId);
    if (iter == listenerPluginMap_.end()) {
        return;
    }
    auto listenerIter = std::remove(messageListenerList_.begin(), messageListenerList_.end(), iter->second);
    if (listenerIter != messageListenerList_.end()) {
        messageListenerList_.erase(listenerIter, messageListenerList_.end());
    }
}

ErrCode ListenerManagerAdapter::StartListener()
{
    for (auto& listener : messageListenerList_) {
        listener->StartListener();
    }
    return ERR_OK;
}

ErrCode ListenerManagerAdapter::StopListener()
{
    for (auto& listener : messageListenerList_) {
        listener->StopListener();
    }
    return ERR_OK;
}

void ListenerManagerAdapter::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{}
}  // namespace DevStandbyMgr
}  // namespace OHOS
