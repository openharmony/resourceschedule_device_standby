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
#include "gtest/gtest.h"
#include "gtest/hwext/gtest-multithread.h"

#include "standby_service_log.h"
#include "json_utils.h"
#include "standby_config_manager.h"
#include "nlohmann/json.hpp"

#include "bundle_manager_helper.h"
#include "app_mgr_helper.h"
#include "ability_manager_helper.h"
#include "background_task_helper.h"
#include "ability_manager_helper.h"

using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    const std::string JSON_KEY = "key";
    const std::string JSON_ERROR_KEY = "error_key";
}
class StandbHelperUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: StandbHelperUnitTest_001
 * @tc.desc: test AppMgrHelper.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbHelperUnitTest, StandbHelperUnitTest_001, TestSize.Level1)
{
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos;
    bool isRunning {false};
    sptr<AppExecFwk::IApplicationStateObserver> observer {nullptr};
    AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos);
    AppMgrHelper::GetInstance()->GetAppRunningStateByBundleName("", isRunning);
    AppMgrHelper::GetInstance()->SubscribeObserver(observer);
    AppMgrHelper::GetInstance()->UnsubscribeObserver(observer);
    AppMgrHelper::GetInstance()->Connect();
    AppMgrHelper::GetInstance()->Connect();
    AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos);
    AppMgrHelper::GetInstance()->GetAppRunningStateByBundleName("", isRunning);
    AppMgrHelper::GetInstance()->SubscribeObserver(observer);
    AppMgrHelper::GetInstance()->UnsubscribeObserver(observer);
    EXPECT_TRUE(allAppProcessInfos.empty());
}

/**
 * @tc.name: StandbHelperUnitTest_002
 * @tc.desc: test BundleManagerHelper.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbHelperUnitTest, StandbHelperUnitTest_002, TestSize.Level1)
{
    int32_t uid {0};
    BundleManagerHelper::GetInstance()->GetClientBundleName(uid);
    AppExecFwk::ApplicationInfo applicationInfo {};
    BundleManagerHelper::GetInstance()->GetApplicationInfo("test",
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, 0, applicationInfo);
    EXPECT_FALSE(applicationInfo.uid > 0);
}

/**
 * @tc.name: StandbHelperUnitTest_003
 * @tc.desc: test BackgroundTaskHelper.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbHelperUnitTest, StandbHelperUnitTest_003, TestSize.Level1)
{
    std::vector<std::shared_ptr<OHOS::BackgroundTaskMgr::ContinuousTaskCallbackInfo>> list;
    BackgroundTaskHelper::GetInstance()->GetContinuousTaskApps(list);
    std::vector<std::shared_ptr<OHOS::BackgroundTaskMgr::TransientTaskAppInfo>> appInfoList;
    BackgroundTaskHelper::GetInstance()->GetTransientTaskApps(appInfoList);
    EXPECT_TRUE(list.empty());
}

/**
 * @tc.name: StandbHelperUnitTest_004
 * @tc.desc: test AbilityManagerHelper.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbHelperUnitTest, StandbHelperUnitTest_004, TestSize.Level1)
{
    std::list<SystemProcessInfo> systemProcessInfos {};
    AbilityManagerHelper::GetInstance()->GetRunningSystemProcess(systemProcessInfos);
    EXPECT_TRUE(systemProcessInfos.empty());
}

/**
 * @tc.name: StandbHelperUnitTest_005
 * @tc.desc: test BundleManagerHelper.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbHelperUnitTest, StandbHelperUnitTest_005, TestSize.Level1)
{
    BundleManagerHelper::GetInstance()->bundleMgr_  = nullptr;
    AppExecFwk::ApplicationInfo appInfo;
    BundleManagerHelper::GetInstance()->GetApplicationInfo("", AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
        0, appInfo);

    std::vector<AppExecFwk::ApplicationInfo> appInfos {};
    BundleManagerHelper::GetInstance()->GetApplicationInfos(AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
        0, appInfos);

    EXPECT_TRUE(appInfos.empty());
}
}  // namespace DevStandbyMgr
}  // namespace OHOS