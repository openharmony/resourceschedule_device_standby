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

#include "timed_task.h"
#include "ability_manager_helper.h"
#include "app_mgr_helper.h"
#include "bundle_manager_helper.h"
#include "common_event_observer.h"

namespace {
    static constexpr char TEST_DEFAULT_BUNDLE[]  = "bundleName";
    bool g_mockGetAllRunningProcesses = true;
    bool g_mockGetRunningSystemProcess = true;
}

void MockGetAllRunningProcesses(bool mockRet)
{
    g_mockGetAllRunningProcesses = mockRet;
}

void MockGetRunningSystemProcess(bool mockRet)
{
    g_mockGetRunningSystemProcess = mockRet;
}

namespace OHOS {
namespace DevStandbyMgr {

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

bool AppMgrHelper::Connect()
{
    return true;
}

std::string BundleManagerHelper::GetClientBundleName(int32_t uid)
{
    return TEST_DEFAULT_BUNDLE;
}

bool BundleManagerHelper::GetApplicationInfo(const std::string &appName, const AppExecFwk::ApplicationFlag flag,
    const int userId, AppExecFwk::ApplicationInfo &appInfo)
{
    appInfo.runningResourcesApply = true;
    return true;
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
}  // namespace DevStandbyMgr
}  // namespace OHOS