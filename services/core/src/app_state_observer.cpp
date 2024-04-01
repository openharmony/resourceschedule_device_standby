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

#include "app_state_observer.h"

#include "app_mgr_constants.h"
#include "app_mgr_helper.h"
#include "standby_service_impl.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
AppStateObserver::AppStateObserver(const std::shared_ptr<AppExecFwk::EventHandler>& handler): handler_(handler) {}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    STANDBYSERVICE_LOGD("process died, uid : %{public}d, pid : %{public}d", processData.uid, processData.pid);
    if (!this->CheckAlivedApp(processData.bundleName)) {
        auto uid = processData.uid;
        auto bundleName = processData.bundleName;
        handler_->PostTask([uid, bundleName]() {
            StandbyServiceImpl::GetInstance()->RemoveAppAllowRecord(uid, bundleName, false);
        });
    }
    handler_->PostTask([uid = processData.uid, pid = processData.pid, bundleName = processData.bundleName]() {
        StandbyServiceImpl::GetInstance()->OnProcessStatusChanged(uid, pid, bundleName, false);
    });
}

bool AppStateObserver::CheckAlivedApp(const std::string &bundleName)
{
    STANDBYSERVICE_LOGD("start check app alive or not");
    bool isRunning {true};
    if (!AppMgrHelper::GetInstance()->GetAppRunningStateByBundleName(bundleName, isRunning)) {
        STANDBYSERVICE_LOGW("connect to app mgr service failed");
        return true;
    }
    return isRunning;
}

void AppStateObserver::OnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    handler_->PostTask([uid = processData.uid, pid = processData.pid, bundleName = processData.bundleName]() {
        StandbyServiceImpl::GetInstance()->OnProcessStatusChanged(uid, pid, bundleName, true);
    });
}

void AppStateObserver::OnApplicationStateChanged(const AppExecFwk::AppStateData &appStateData)
{
    if (!(appStateData.uid > 0 && appStateData.bundleName.size() > 0)) {
        STANDBYSERVICE_LOGD("%{public}s : validate app state data failed!", __func__);
        return;
    }
    auto uid = appStateData.uid;
    auto bundleName = appStateData.bundleName;
    auto state = appStateData.state;
    STANDBYSERVICE_LOGD("app is terminated, uid: %{public}d, bunddlename: %{public}s", uid, bundleName.c_str());
    if (state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_TERMINATED) || state ==
        static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_END)) {
        handler_->PostTask([uid, bundleName]() {
            StandbyServiceImpl::GetInstance()->RemoveAppAllowRecord(uid, bundleName, false);
        });
    }
}
}  // namespace DevStandbyMgr
}  // namespace OHOS