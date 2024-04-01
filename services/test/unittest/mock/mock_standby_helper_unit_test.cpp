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

#include "bundle_manager_helper.h"
#include "app_mgr_helper.h"
#include "ability_manager_helper.h"
#ifdef ENABLE_BACKGROUND_TASK_MGR
#include "background_task_helper.h"
#endif
#include "standby_service_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DevStandbyMgr {
class MockStandbyHelperUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: MockStandbyHelperUnitTest_001
 * @tc.desc: test AppMgrHelper.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MockStandbyHelperUnitTest, MockStandbyHelperUnitTest_001, TestSize.Level1)
{
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos;
    bool isRunning {false};
    sptr<AppExecFwk::IApplicationStateObserver> observer {nullptr};
    AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos);
    AppMgrHelper::GetInstance()->GetAppRunningStateByBundleName("", isRunning);
    AppMgrHelper::GetInstance()->SubscribeObserver(observer);
    AppMgrHelper::GetInstance()->UnsubscribeObserver(observer);
    AppMgrHelper::GetInstance()->Connect();
    EXPECT_TRUE(allAppProcessInfos.empty());
}

/**
 * @tc.name: MockStandbyHelperUnitTest_002
 * @tc.desc: test BundleManagerHelper.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MockStandbyHelperUnitTest, MockStandbyHelperUnitTest_002, TestSize.Level1)
{
    int32_t uid {0};
    BundleManagerHelper::GetInstance()->GetClientBundleName(uid);

    AppExecFwk::ApplicationInfo appInfo;
    BundleManagerHelper::GetInstance()->GetApplicationInfo("", AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
        0, appInfo);

    std::vector<AppExecFwk::ApplicationInfo> appInfos {};
    BundleManagerHelper::GetInstance()->GetApplicationInfos(AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
        0, appInfos);

    EXPECT_FALSE(appInfo.runningResourcesApply);
}

#ifdef ENABLE_BACKGROUND_TASK_MGR
/**
 * @tc.name: MockStandbyHelperUnitTest_003
 * @tc.desc: test BackgroundTaskHelper.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MockStandbyHelperUnitTest, MockStandbyHelperUnitTest_003, TestSize.Level1)
{
    std::vector<std::shared_ptr<OHOS::BackgroundTaskMgr::ContinuousTaskCallbackInfo>> list;
    BackgroundTaskHelper::GetInstance()->GetContinuousTaskApps(list);
    EXPECT_TRUE(list.empty());
    std::vector<std::shared_ptr<OHOS::BackgroundTaskMgr::TransientTaskAppInfo>> appInfoList;
    BackgroundTaskHelper::GetInstance()->GetTransientTaskApps(appInfoList);
}
#endif
}  // namespace DevStandbyMgr
}  // namespace OHOS
