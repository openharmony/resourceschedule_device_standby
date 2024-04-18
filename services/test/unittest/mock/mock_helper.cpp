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

#include "timed_task.h"
#include "ability_manager_helper.h"
#include "app_mgr_helper.h"
#include "bundle_manager_helper.h"
#include "common_event_observer.h"
#ifdef ENABLE_BACKGROUND_TASK_MGR
#include "background_task_helper.h"
#endif
#include "ibundle_manager_helper.h"

namespace {
    static constexpr char TEST_DEFAULT_BUNDLE[]  = "bundleName";
    static constexpr int32_t EXEMPT_ALL_RESOURCES = 100;
    bool g_mockGetAllRunningProcesses = true;
    bool g_mockGetRunningSystemProcess = true;
    bool g_mockGetBackgroundTask = true;
    bool g_mockSubscribeObserver = true;
}

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    std::shared_ptr<IBundleManagerHelper> bundleManagerHelperMock;
}

void SetBundleManagerHelper(std::shared_ptr<IBundleManagerHelper> mock)
{
    bundleManagerHelperMock = mock;
}

void CleanBundleManagerHelper()
{
    bundleManagerHelperMock.reset();
}

bool BundleManagerHelper::GetApplicationInfo(const std::string &appName, const AppExecFwk::ApplicationFlag flag,
    const int userId, AppExecFwk::ApplicationInfo &appInfo)
{
    bool ret {true};
    if (bundleManagerHelperMock) {
        ret = bundleManagerHelperMock->GetApplicationInfo(appName, flag, userId, appInfo);
    } else {
        appInfo.resourcesApply = { EXEMPT_ALL_RESOURCES };
    }
    return ret;
}

uint64_t TimedTask::CreateTimer(bool repeat, uint64_t interval, bool isExact, bool isIdle,
    const std::function<void()>& callBack)
{
    return 1;
}

bool TimedTask::StartDayNightSwitchTimer(uint64_t& timeId)
{
    return true;
}

bool AbilityManagerHelper::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
    systemProcessInfos.emplace_back(SystemProcessInfo{});
    return g_mockGetRunningSystemProcess;
}

bool AppMgrHelper::GetAllRunningProcesses(std::vector<AppExecFwk::RunningProcessInfo>& allAppProcessInfos)
{
    allAppProcessInfos.emplace_back(AppExecFwk::RunningProcessInfo{});
    return g_mockGetAllRunningProcesses;
}

bool AppMgrHelper::GetForegroundApplications(std::vector<AppExecFwk::AppStateData>& fgApps)
{
    return g_mockGetAllRunningProcesses;
}


bool AppMgrHelper::Connect()
{
    return true;
}

bool AppMgrHelper::GetAppRunningStateByBundleName(const std::string &bundleName, bool& isRunning)
{
    isRunning = true;
    return true;
}

std::string BundleManagerHelper::GetClientBundleName(int32_t uid)
{
    return TEST_DEFAULT_BUNDLE;
}

bool BundleManagerHelper::Connect()
{
    return true;
}

bool CommonEventObserver::Subscribe()
{
    return true;
}

bool CommonEventObserver::Unsubscribe()
{
    return true;
}

#ifdef ENABLE_BACKGROUND_TASK_MGR
bool BackgroundTaskHelper::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    return g_mockGetBackgroundTask;
}

bool BackgroundTaskHelper::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    return g_mockGetBackgroundTask;
}
#endif

bool AppMgrHelper::SubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer)
{
    return g_mockSubscribeObserver;
}

bool AppMgrHelper::UnsubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer)
{
    return g_mockSubscribeObserver;
}

void IBundleManagerHelper::MockGetAllRunningProcesses(bool mockRet)
{
    g_mockGetAllRunningProcesses = mockRet;
}

void IBundleManagerHelper::MockGetRunningSystemProcess(bool mockRet)
{
    g_mockGetRunningSystemProcess = mockRet;
}

void IBundleManagerHelper::MockGetBackgroundTask(bool mockRet)
{
    g_mockGetBackgroundTask = mockRet;
}

void IBundleManagerHelper::MockSubscribeObserver(bool mockRet)
{
    g_mockSubscribeObserver = mockRet;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS