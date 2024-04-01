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

#define private public
#define protected public

#include "gtest/gtest.h"
#include "gtest/hwext/gtest-multithread.h"
#include "system_ability_definition.h"

#include "running_lock_strategy.h"
#ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
#include "network_strategy.h"
#include "base_network_strategy.h"
#endif
#include "standby_messsage.h"
#include "common_constant.h"
#include "want.h"

using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace DevStandbyMgr {
class StandbyPluginStrategyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override {}
    void TearDown() override {}
};

void StandbyPluginStrategyTest::TearDownTestCase()
{
}

void StandbyPluginStrategyTest::SetUpTestCase()
{
}

/**
 * @tc.name: StandbyPluginStrategyTest_001
 * @tc.desc: test GetAndCreateAppInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_001, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    int32_t uid = 1;
    int32_t pid = 1;
    std::string bundleName = "defaultBundleName";
    std::string mapKey = std::to_string(uid) + "_" + bundleName;
    struct ProxiedProcInfo procInfo = {
        bundleName,
        uid,
        {pid}
    };
    runningLockStrategy->proxiedAppInfo_.emplace(mapKey, procInfo);
    runningLockStrategy->GetAndCreateAppInfo(uid, pid, bundleName);

    uid = 2;
    runningLockStrategy->GetAndCreateAppInfo(uid, pid, bundleName);
    EXPECT_NE(runningLockStrategy, nullptr);
}

/**
 * @tc.name: StandbyPluginStrategyTest_002
 * @tc.desc: test GetExemptionConfigForApp.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_002, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    uint32_t uid = 1;
    uint32_t pid = 1;
    std::string bundleName = "defaultBundleName";
    struct ProxiedProcInfo procInfo = {
        bundleName,
        uid,
        {pid}
    };
    runningLockStrategy->GetExemptionConfigForApp(procInfo, bundleName);
    EXPECT_NE(runningLockStrategy, nullptr);
}

/**
 * @tc.name: StandbyPluginStrategyTest_003
 * @tc.desc: test ProxyRunningLockList.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_003, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    bool isProxied = true;
    std::vector<std::pair<int32_t, int32_t>> proxiedAppList;
    int32_t a = 1;
    int32_t b = 1;
    proxiedAppList.emplace_back(std::make_pair(a, b));
    runningLockStrategy->isIdleMaintence_ = false;
    runningLockStrategy->ProxyRunningLockList(isProxied, proxiedAppList);

    runningLockStrategy->isIdleMaintence_ = true;
    runningLockStrategy->ProxyRunningLockList(isProxied, proxiedAppList);
    EXPECT_NE(runningLockStrategy, nullptr);
}

/**
 * @tc.name: StandbyPluginStrategyTest_004
 * @tc.desc: test HandleProcessStatusChanged.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_004, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    int32_t uid = 1;
    int32_t pid = 1;
    std::string bundleName = "defaultBundleName";
    StandbyMessage standbyMessage {StandbyMessageType::PROCESS_STATE_CHANGED};
    AAFwk::Want want = AAFwk::Want();
    standbyMessage.want_ = want;
    standbyMessage.want_->SetParam("uid", uid);
    standbyMessage.want_->SetParam("pid", pid);
    standbyMessage.want_->SetParam("name", bundleName);
    standbyMessage.want_->SetParam("isCreated", true);

    runningLockStrategy->isProxied_ = true;
    std::string mapKey = std::to_string(uid) + "_" + bundleName;
    struct ProxiedProcInfo procInfo = {
        bundleName,
        uid,
        {pid}
    };
    runningLockStrategy->proxiedAppInfo_.emplace(mapKey, procInfo);
    runningLockStrategy->HandleProcessStatusChanged(standbyMessage);

    uid = 2;
    standbyMessage.want_->SetParam("uid", uid);
    standbyMessage.want_->SetParam("isCreated", false);
    runningLockStrategy->HandleProcessStatusChanged(standbyMessage);
    EXPECT_NE(runningLockStrategy, nullptr);
}

/**
 * @tc.name: StandbyPluginStrategyTest_005
 * @tc.desc: test GetBackgroundTaskApp.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_005, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    EXPECT_EQ(runningLockStrategy->GetBackgroundTaskApp(), ERR_OK);
}

/**
 * @tc.name: StandbyPluginStrategyTest_006
 * @tc.desc: test GetBackgroundTaskApp.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_006, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    EXPECT_EQ(runningLockStrategy->GetForegroundApplications(), ERR_OK);
}

/**
 * @tc.name: StandbyPluginStrategyTest_007
 * @tc.desc: test GetWorkSchedulerTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_007, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    EXPECT_EQ(runningLockStrategy->GetWorkSchedulerTask(), ERR_OK);
}

/**
 * @tc.name: StandbyPluginStrategyTest_008
 * @tc.desc: test GetAllRunningAppInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_008, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    EXPECT_EQ(runningLockStrategy->GetAllRunningAppInfo(), ERR_OK);
}

/**
 * @tc.name: StandbyPluginStrategyTest_009
 * @tc.desc: test OnDestroy.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_009, TestSize.Level1)
{
    auto runningLockStrategy = std::make_shared<RunningLockStrategy>();
    runningLockStrategy->isProxied_ = true;
    runningLockStrategy->isIdleMaintence_ = true;
    EXPECT_EQ(runningLockStrategy->OnDestroy(), ERR_OK);

    runningLockStrategy->isIdleMaintence_ = false;
    EXPECT_EQ(runningLockStrategy->OnDestroy(), ERR_OK);
}

#ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
/**
 * @tc.name: StandbyPluginStrategyTest_010
 * @tc.desc: test ResetFirewallStatus.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_010, TestSize.Level1)
{
    auto baseNetworkStrategy = std::make_shared<NetworkStrategy>();
    StandbyMessage standbyMessage {StandbyMessageType::SYS_ABILITY_STATUS_CHANGED};

    baseNetworkStrategy->isFirewallEnabled_ = true;
    baseNetworkStrategy->isIdleMaintence_ = true;
    baseNetworkStrategy->ResetFirewallStatus(standbyMessage);
    
    baseNetworkStrategy->isFirewallEnabled_ = true;
    baseNetworkStrategy->isIdleMaintence_ = false;
    AAFwk::Want want = AAFwk::Want();
    standbyMessage.want_ = want;
    standbyMessage.want_->SetParam(SA_STATUS, false);
    standbyMessage.want_->SetParam(SA_ID, WORK_SCHEDULE_SERVICE_ID);
    baseNetworkStrategy->ResetFirewallStatus(standbyMessage);

    standbyMessage.want_->SetParam(SA_ID, BACKGROUND_TASK_MANAGER_SERVICE_ID);
    baseNetworkStrategy->ResetFirewallStatus(standbyMessage);

    standbyMessage.want_->SetParam(SA_ID, DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID);
    baseNetworkStrategy->ResetFirewallStatus(standbyMessage);
    EXPECT_NE(baseNetworkStrategy, nullptr);
}

/**
 * @tc.name: StandbyPluginStrategyTest_011
 * @tc.desc: test GetExemptionConfigForApp.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_011, TestSize.Level1)
{
    auto baseNetworkStrategy = std::make_shared<NetworkStrategy>();
    std::string bundleName = "defaultBundleName";
    NetLimtedAppInfo appInfo {bundleName};

    baseNetworkStrategy->GetExemptionConfigForApp(appInfo, bundleName);
    EXPECT_NE(baseNetworkStrategy, nullptr);
}

/**
 * @tc.name: StandbyPluginStrategyTest_012
 * @tc.desc: test GetExemptionConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_012, TestSize.Level1)
{
    auto baseNetworkStrategy = std::make_shared<NetworkStrategy>();
    baseNetworkStrategy->GetExemptionConfig();
    EXPECT_NE(baseNetworkStrategy, nullptr);
}

/**
 * @tc.name: StandbyPluginStrategyTest_013
 * @tc.desc: test GetExemptionConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginStrategyTest, StandbyPluginStrategyTest_013, TestSize.Level1)
{
    auto baseNetworkStrategy = std::make_shared<NetworkStrategy>();
    int32_t uid = 1;
    uint8_t flag = ExemptionTypeFlag::UNRESTRICTED;
    NetLimtedAppInfo appInfo {"defaulBundleName"};
    baseNetworkStrategy->netLimitedAppInfo_.emplace(uid, appInfo);
    baseNetworkStrategy->AddExemptionFlagByUid(uid, flag);

    uid = 2;
    baseNetworkStrategy->AddExemptionFlagByUid(uid, flag);
    EXPECT_NE(baseNetworkStrategy, nullptr);
}
#endif // STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
}  // namespace DevStandbyMgr
}  // namespace OHOS
