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

#include "background_task_listener.h"
#include "standby_service_impl.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
BackgroundTaskListener::BackgroundTaskListener()
{
    bgTaskListenerImpl_ = std::make_unique<BgTaskListenerImpl>();
}

ErrCode BackgroundTaskListener::StartListener()
{
    if (!bgTaskListenerImpl_) {
        STANDBYSERVICE_LOGE("backgroundTaskListener is nullptr");
        return ERR_BGTASK_LISTENER_NULL;
    }
    if (BackgroundTaskMgrHelper::SubscribeBackgroundTask(*bgTaskListenerImpl_) != OHOS::ERR_OK) {
        STANDBYSERVICE_LOGE("SubscribeBackgroundTask failed");
        return ERR_REGISTER_BACKGROUND_TASK_FAILED;
    }
    STANDBYSERVICE_LOGI("backgroundTaskListener start listener");
    return ERR_OK;
}

ErrCode BackgroundTaskListener::StopListener()
{
    if (!bgTaskListenerImpl_) {
        STANDBYSERVICE_LOGE("backgroundTaskListener is nullptr");
        return ERR_BGTASK_LISTENER_NULL;
    }
    if (BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*bgTaskListenerImpl_) != OHOS::ERR_OK) {
        STANDBYSERVICE_LOGE("UnsubscribeBackgroundTask failed");
        return ERR_UNREGISTER_BACKGROUND_TASK_FAILED;
    }
    STANDBYSERVICE_LOGI("backgroundTaskListener stop listener");
    return ERR_OK;
}

BackgroundTaskListener::BgTaskListenerImpl::BgTaskListenerImpl()
{
    handler_ = StandbyServiceImpl::GetInstance()->GetHandler();
}

void BackgroundTaskListener::BgTaskListenerImpl::OnAppTransientTaskStart(const
    std::shared_ptr<TransientTaskAppInfo>& info)
{
    STANDBYSERVICE_LOGD("Transient start called, uid is %{public}d, bundle name is %{public}s",
        info->GetUid(), info->GetPackageName().c_str());
    OnTaskStatusChanged(TRANSIENT_TASK, true, info->GetUid(), info->GetPid(), info->GetPackageName());
}

void BackgroundTaskListener::BgTaskListenerImpl::OnAppTransientTaskEnd(const
    std::shared_ptr<TransientTaskAppInfo>& info)
{
    STANDBYSERVICE_LOGD("Transient stop called, uid is %{public}d, bundle name is %{public}s",
        info->GetUid(), info->GetPackageName().c_str());
    OnTaskStatusChanged(TRANSIENT_TASK, false, info->GetUid(), info->GetPid(), info->GetPackageName());
}

void BackgroundTaskListener::BgTaskListenerImpl::OnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    STANDBYSERVICE_LOGD("Continuous start called, uid is %{public}d", continuousTaskCallbackInfo->GetCreatorUid());
    OnTaskStatusChanged(CONTINUOUS_TASK, true, continuousTaskCallbackInfo->GetCreatorUid(),
        continuousTaskCallbackInfo->GetCreatorPid(), "");
}

void BackgroundTaskListener::BgTaskListenerImpl::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    STANDBYSERVICE_LOGD("Continuous stop called, uid is %{public}d", continuousTaskCallbackInfo->GetCreatorUid());
    OnTaskStatusChanged(CONTINUOUS_TASK, false, continuousTaskCallbackInfo->GetCreatorUid(),
        continuousTaskCallbackInfo->GetCreatorPid(), "");
}

void BackgroundTaskListener::BgTaskListenerImpl::OnTaskStatusChanged(const std::string& type, bool started,
    int32_t uid, int32_t pid, const std::string& bundleName)
{
    StandbyMessage standbyMessage {StandbyMessageType::BG_TASK_STATUS_CHANGE};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam(BG_TASK_TYPE, type);
    standbyMessage.want_->SetParam(BG_TASK_STATUS, started);
    standbyMessage.want_->SetParam(BG_TASK_UID, uid);
    standbyMessage.want_->SetParam(BG_TASK_BUNDLE_NAME, bundleName);
    StandbyServiceImpl::GetInstance()->DispatchEvent(standbyMessage);
}
} // OHOS
} // DevStandbyMgr