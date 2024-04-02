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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_APP_STATE_OBSERVER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_APP_STATE_OBSERVER_H

#include "app_mgr_interface.h"
#include "application_state_observer_stub.h"
#include "event_handler.h"
#include "iremote_object.h"

namespace OHOS {
namespace DevStandbyMgr {
class AppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
public:

    /**
     * @brief constructor of AppStateObserver.
     */
    explicit AppStateObserver(const std::shared_ptr<AppExecFwk::EventHandler>& handler);
    /**
     * @brief Callback when process died.
     *
     * @param processData process data.
     */
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override;

    /**
     * Will be called when the process start.
     *
     * @param processData Process data.
     */
    void OnProcessCreated(const AppExecFwk::ProcessData &processData) override;

    /**
     * Application foreground state changed callback.
     *
     * @param appStateData Application Process data.
     */
    void OnApplicationStateChanged(const AppExecFwk::AppStateData &appStateData) override;

private:
    bool CheckAlivedApp(const std::string &bundleName);

private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_STATE_OBSERVER_H