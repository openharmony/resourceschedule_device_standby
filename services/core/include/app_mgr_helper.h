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
#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_INCLUDE_APP_MGR_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_INCLUDE_APP_MGR_HELPER_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include "app_mgr_interface.h"
#include "app_mgr_proxy.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "single_instance.h"

namespace OHOS {
namespace DevStandbyMgr {
class AppMgrHelper {
DECLARE_SINGLE_INSTANCE(AppMgrHelper);
public:
    /**
     * @brief Get the All Running Processes info.
     *
     * @param allAppProcessInfos process info of applications.
     * @return true if subscribe successfully.
     */
    bool GetAllRunningProcesses(std::vector<AppExecFwk::RunningProcessInfo>& allAppProcessInfos);

    /**
     * @brief Get the App Running State By Bundle Name object.
     */
    bool GetAppRunningStateByBundleName(const std::string &bundleName, bool& isRunning);

    /**
     * Get Foreground Applications.
     */
    bool GetForegroundApplications(std::vector<AppExecFwk::AppStateData> &fgApps);

    /**
     * @brief Subscribe AppStateObserver.
     *
     * @param observer app state observer.
     * @return true if subscribe successfully.
     */
    bool SubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer);

    /**
     * @brief Unubscribe AppStateObserver.
     *
     * @param observer app state observer.
     * @return true if unsubscribe successfully.
     */
    bool UnsubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer);

private:
    bool Connect();

private:
    sptr<AppExecFwk::IAppMgr> appMgrProxy_ {nullptr};
    std::mutex connectMutex_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS

#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_INCLUDE_APP_MGR_HELPER_H