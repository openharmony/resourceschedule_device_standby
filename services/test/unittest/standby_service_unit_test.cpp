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
#include <functional>
#include <chrono>
#include <thread>
#include <message_parcel.h>
#include <climits>
#include <dlfcn.h>

#include "gtest/gtest.h"
#include "gtest/hwext/gtest-multithread.h"
#include "singleton.h"

#include "device_standby_switch.h"
#include "time_provider.h"
#include "common_event_support.h"
#include "common_event_observer.h"

#include "state_manager_adapter.h"
#include "constraint_manager_adapter.h"
#include "istandby_ipc_inteface_code.h"
#include "listener_manager_adapter.h"
#include "strategy_manager_adapter.h"

#include "standby_state.h"
#include "allow_type.h"
#include "standby_ipc_interface_code.h"
#include "standby_service_client.h"
#include "standby_service.h"
#include "ability_manager_helper.h"
#include "standby_service_impl.h"
#include "standby_state_subscriber.h"
#include "standby_state_subscriber.h"
#include "standby_service_subscriber_stub.h"
#include "bundle_manager_helper.h"
#include "standby_config_manager.h"
using namespace testing::ext;
using namespace testing::mt;

extern void MockPublishCommonEvent(bool mockRet);
extern void MockSubscribeCommonEvent(bool mockRet);
extern void MockSubscribeCommonEvent(bool mockRet);
extern void MockGetAllRunningProcesses(bool mockRet);
extern void MockGetRunningSystemProcess(bool mockRet);
extern void MockGetTokenTypeFlag(bool mockRet);
extern void MockStartTimer(bool mockRet);

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    const uint32_t ALL_DEPENDS_READY = 31;
    constexpr int32_t DEFAULT_UID = 0;
    const std::string DEFAULT_BUNDLENAME = "test";
    const std::string DEFAULT_KEY = "0_test";
    const vector<std::string> COMMON_EVENT_LIST = {
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED,
        EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_REMOVED,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED,
        EventFwk::CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED,
        EventFwk::CommonEventSupport::COMMON_EVENT_NITZ_TIMEZONE_CHANGED,
        EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED,
        EventFwk::CommonEventSupport::COMMON_EVENT_NITZ_TIME_CHANGED
    };
    constexpr int32_t SLEEP_TIMEOUT = 500;
}

class StandbyServiceUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override {}
    void TearDown() override;

    inline static void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMEOUT));
    }
};

void StandbyServiceUnitTest::SetUpTestCase()
{
    StandbyServiceImpl::GetInstance()->Init();

    StandbyServiceImpl::GetInstance()->constraintManager_ = std::make_shared<ConstraintManagerAdapter>();
    StandbyServiceImpl::GetInstance()->listenerManager_ = std::make_shared<ListenerManagerAdapter>();
    StandbyServiceImpl::GetInstance()->strategyManager_ = std::make_shared<StrategyManagerAdapter>();
    StandbyServiceImpl::GetInstance()->standbyStateManager_ = std::make_shared<StateManagerAdapter>();
    StandbyServiceImpl::GetInstance()->InitReadyState();
    SleepForFC();
}

void StandbyServiceUnitTest::TearDown()
{
    SleepForFC();
    StandbyServiceImpl::GetInstance()->allowInfoMap_.clear();
}

void StandbyServiceUnitTest::TearDownTestCase()
{
    SleepForFC();
    if (StandbyServiceImpl::GetInstance()->handler_) {
        StandbyServiceImpl::GetInstance()->handler_->RemoveAllEvents();
        auto runner = StandbyServiceImpl::GetInstance()->handler_->GetEventRunner();
        if (runner) {
            runner->Stop();
            runner = nullptr;
        }
        StandbyServiceImpl::GetInstance()->handler_ = nullptr;
    }
    StandbyServiceImpl::GetInstance()->UnInit();
}

/**
 * @tc.name: StandbyServiceUnitTest_001
 * @tc.desc: test OnStart of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_001, TestSize.Level1)
{
    StandbyService::GetInstance()->state_ = ServiceRunningState::STATE_RUNNING;
    StandbyService::GetInstance()->OnStart();
    EXPECT_NE(StandbyServiceImpl::GetInstance()->handler_, nullptr);
    StandbyServiceImpl::GetInstance()->InitReadyState();
    SleepForFC();
    EXPECT_EQ(StandbyService::GetInstance()->state_, ServiceRunningState::STATE_RUNNING);
}

/**
 * @tc.name: StandbyServiceUnitTest_002
 * @tc.desc: test OnAddSystemAbility of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_002, TestSize.Level1)
{
    StandbyService::GetInstance()->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(TIME_SERVICE_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(ABILITY_MGR_SERVICE_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID + 1, "");
    StandbyService::GetInstance()->OnRemoveSystemAbility(POWER_MANAGER_SERVICE_ID + 1, "");
    SleepForFC();
    StandbyService::GetInstance()->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID + 1, "");
    StandbyService::GetInstance()->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "");
    StandbyService::GetInstance()->OnRemoveSystemAbility(TIME_SERVICE_ID, "");
    StandbyService::GetInstance()->OnRemoveSystemAbility(ABILITY_MGR_SERVICE_ID, "");
    StandbyService::GetInstance()->OnRemoveSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, "");
    StandbyService::GetInstance()->OnRemoveSystemAbility(POWER_MANAGER_SERVICE_ID, "");
    StandbyService::GetInstance()->OnRemoveSystemAbility(POWER_MANAGER_SERVICE_ID + 1, "");
    StandbyService::GetInstance()->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(TIME_SERVICE_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(ABILITY_MGR_SERVICE_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, "");
    StandbyService::GetInstance()->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
    EXPECT_EQ(StandbyService::GetInstance()->dependsReady_, ALL_DEPENDS_READY);
    StandbyServiceImpl::GetInstance()->InitReadyState();
}

/**
 * @tc.name: StandbyServiceUnitTest_003
 * @tc.desc: test state not start of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_003, TestSize.Level1)
{
    StandbyService::GetInstance()->state_ = ServiceRunningState::STATE_NOT_START;
    sptr<IStandbyServiceSubscriber> subscriber = new (std::nothrow) StandbyServiceSubscriberStub();
    StandbyService::GetInstance()->SubscribeStandbyCallback(subscriber);
    StandbyService::GetInstance()->UnsubscribeStandbyCallback(subscriber);

    sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest();
    EXPECT_NE(StandbyService::GetInstance()->ApplyAllowResource(resourceRequest), ERR_OK);
    EXPECT_NE(StandbyService::GetInstance()->UnapplyAllowResource(resourceRequest), ERR_OK);

    std::vector<AllowInfo> allowInfoList;
    EXPECT_NE(StandbyService::GetInstance()->GetAllowList(AllowType::NETWORK, allowInfoList, 0), ERR_OK);
    bool isStandby {false};
    EXPECT_NE(StandbyService::GetInstance()->IsDeviceInStandby(isStandby), ERR_OK);
    StandbyService::GetInstance()->state_ = ServiceRunningState::STATE_RUNNING;
    EXPECT_TRUE(StandbyServiceImpl::GetInstance()->isServiceReady_.load());
    StandbyService::GetInstance()->SubscribeStandbyCallback(subscriber);
    StandbyService::GetInstance()->UnsubscribeStandbyCallback(subscriber);
    StandbyService::GetInstance()->ApplyAllowResource(resourceRequest);
    StandbyService::GetInstance()->UnapplyAllowResource(resourceRequest);
    EXPECT_EQ(StandbyService::GetInstance()->GetAllowList(AllowType::NETWORK, allowInfoList, 0), ERR_OK);
    EXPECT_EQ(StandbyService::GetInstance()->IsDeviceInStandby(isStandby), ERR_OK);
}

/**
 * @tc.name: StandbyServiceUnitTest_004
 * @tc.desc: test unready not start of StandbyServiceImpl.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_004, TestSize.Level1)
{
    StandbyServiceImpl::GetInstance()->isServiceReady_ = false;
    StandbyServiceImpl::GetInstance()->RemoveAppAllowRecord(DEFAULT_UID, DEFAULT_BUNDLENAME, true);
    sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest();
    StandbyServiceImpl::GetInstance()->ApplyAllowResource(resourceRequest);
    StandbyServiceImpl::GetInstance()->UnapplyAllowResource(resourceRequest);
    std::vector<AllowInfo> allowInfoList;
    StandbyServiceImpl::GetInstance()->GetAllowList(AllowType::NETWORK, allowInfoList, 0);
    bool isStandby {false};
    EXPECT_NE(StandbyServiceImpl::GetInstance()->IsDeviceInStandby(isStandby), ERR_OK);
    std::vector<std::string> argsInStr {};
    std::string result;
    StandbyServiceImpl::GetInstance()->ShellDump(argsInStr, result);
    StandbyServiceImpl::GetInstance()->isServiceReady_ = true;
}

/**
 * @tc.name: StandbyServiceUnitTest_005
 * @tc.desc: test shelldump not start of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_005, TestSize.Level1)
{
    std::vector<std::u16string> args{};
    std::vector<std::string> argsInStr {};
    std::string result;
    StandbyServiceImpl::GetInstance()->ShellDump(argsInStr, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-h"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-v"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-D"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-E"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-E", "0", "false"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-E", "0", "true"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--apply"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--unapply"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--get"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--apply", "0", "test", "127", "100", "0"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--apply", "0", "test", "127", "100", "1"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--unapply", "0", "test", "127", "100", "0"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--unapply", "0", "test", "127", "100", "1"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--get", "127", "true"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-A", "--get", "127", "false"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-S"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-S", "--repeat"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-S", "--motion"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-S", "--default"}, result);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-D"}, result);

    auto allowRecord = std::make_shared<AllowRecord>(0, 0, "name", AllowType::NETWORK);
    allowRecord->allowTimeList_.emplace_back(AllowTime{0, INT64_MAX, "reason"});
    StandbyServiceImpl::GetInstance()->allowInfoMap_.emplace(DEFAULT_KEY, allowRecord);
    StandbyServiceImpl::GetInstance()->ShellDumpInner({"-D"}, result);
    SleepForFC();
    EXPECT_NE(StandbyService::GetInstance()->Dump(-1, args), ERR_OK);
}

/**
 * @tc.name: StandbyServiceUnitTest_006
 * @tc.desc: test init of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_006, TestSize.Level1)
{
    EXPECT_NE(StandbyServiceImpl::GetInstance()->RegisterPlugin("test_standby.z.so"), ERR_OK);
    EXPECT_NE(StandbyServiceImpl::GetInstance()->RegisterPlugin("libstandby_utils_policy.z.so"), ERR_OK);
}

/**
 * @tc.name: StandbyServiceUnitTest_007
 * @tc.desc: test init of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_007, TestSize.Level1)
{
    EXPECT_NE(StandbyServiceImpl::GetInstance()->RegisterPlugin("test_standby.z.so"), ERR_OK);
    EXPECT_NE(StandbyServiceImpl::GetInstance()->RegisterPlugin("libstandby_utils_policy.z.so"), ERR_OK);
}

/**
 * @tc.name: StandbyServiceUnitTest_008
 * @tc.desc: test ParsePersistentData of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_008, TestSize.Level1)
{
    StandbyServiceImpl::GetInstance()->ParsePersistentData();
    auto allowRecord = std::make_shared<AllowRecord>(0, 0, "name", AllowType::NETWORK);
    allowRecord->allowTimeList_.emplace_back(AllowTime{0, INT64_MAX, "reason"});
    allowRecord = std::make_shared<AllowRecord>(-1, -1, "test", AllowType::NETWORK);
    allowRecord->allowTimeList_.emplace_back(AllowTime{-1, INT64_MAX, "test"});
    allowRecord = std::make_shared<AllowRecord>(-1, -1, "test", AllowType::NETWORK);
    allowRecord->allowTimeList_.emplace_back(AllowTime{-1, INT64_MAX, "test"});
    StandbyServiceImpl::GetInstance()->allowInfoMap_.emplace(DEFAULT_KEY, allowRecord);
    StandbyServiceImpl::GetInstance()->DumpPersistantData();
    StandbyServiceImpl::GetInstance()->ParsePersistentData();
    StandbyServiceImpl::GetInstance()->RecoverTimeLimitedTask();
    allowRecord->allowTimeList_.clear();
    StandbyServiceImpl::GetInstance()->DumpPersistantData();
    StandbyServiceImpl::GetInstance()->ParsePersistentData();
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->allowInfoMap_.size(), 0);
    MockGetAllRunningProcesses(false);
    StandbyServiceImpl::GetInstance()->ParsePersistentData();
    MockGetAllRunningProcesses(true);
}

/**
 * @tc.name: StandbyServiceUnitTest_009
 * @tc.desc: test ResetTimeObserver of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_009, TestSize.Level1)
{
    StandbyServiceImpl::GetInstance()->ResetTimeObserver();
    EXPECT_NE(StandbyServiceImpl::GetInstance()->dayNightSwitchTimerId_, 0);
}

/**
 * @tc.name: StandbyServiceUnitTest_010
 * @tc.desc: test UnInit of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_010, TestSize.Level1)
{
    StandbyServiceImpl::GetInstance()->registerPlugin_ = nullptr;
    StandbyServiceImpl::GetInstance()->UninitReadyState();
    StandbyServiceImpl::GetInstance()->UnInit();
    StandbyServiceImpl::GetInstance()->RegisterPlugin(StandbyConfigManager::GetInstance()
        ->GetPluginName());
    StandbyServiceImpl::GetInstance()->InitReadyState();
    SleepForFC();
    EXPECT_NE(StandbyServiceImpl::GetInstance()->registerPlugin_, nullptr);
}

/**
 * @tc.name: StandbyServiceUnitTest_011
 * @tc.desc: test RemoveAppAllowRecord of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_011, TestSize.Level1)
{
    StandbyServiceImpl::GetInstance()->RemoveAppAllowRecord(DEFAULT_UID, DEFAULT_BUNDLENAME, true);
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->allowInfoMap_.size(), 0);
}

/**
 * @tc.name: StandbyServiceUnitTest_012
 * @tc.desc: test CheckCallerPermission of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_012, TestSize.Level1)
{
    StandbyServiceImpl::GetInstance()->CheckCallerPermission(ReasonCodeEnum::REASON_NATIVE_API);
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->CheckCallerPermission(ReasonCodeEnum::REASON_APP_API), ERR_OK);
    MockGetTokenTypeFlag(false);
    StandbyServiceImpl::GetInstance()->CheckCallerPermission(ReasonCodeEnum::REASON_NATIVE_API);
    StandbyServiceImpl::GetInstance()->CheckCallerPermission(ReasonCodeEnum::REASON_APP_API);
    MockGetTokenTypeFlag(true);
}

/**
 * @tc.name: StandbyServiceUnitTest_013
 * @tc.desc: test ApplyAllowResource of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_013, TestSize.Level1)
{
    sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest();
    StandbyServiceImpl::GetInstance()->ApplyAllowResource(resourceRequest);
    SleepForFC();
    StandbyServiceImpl::GetInstance()->ApplyAllowResInner(resourceRequest, -1);
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->allowInfoMap_.size(), 0);
}

/**
 * @tc.name: StandbyServiceUnitTest_014
 * @tc.desc: test UpdateRecord of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, UpdateRecord_014, TestSize.Level1)
{
    std::shared_ptr<AllowRecord> allowRecord = std::make_shared<AllowRecord>();
    sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest();
    StandbyServiceImpl::GetInstance()->UpdateRecord(allowRecord, resourceRequest);
    SleepForFC();
    StandbyServiceImpl::GetInstance()->UpdateRecord(allowRecord, resourceRequest);
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->allowInfoMap_.size(), 0);
}

/**
 * @tc.name: StandbyServiceUnitTest_015
 * @tc.desc: test UpdateRecord of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_015, TestSize.Level1)
{
    std::shared_ptr<AllowRecord> allowRecord = std::make_shared<AllowRecord>();
    sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest(MAX_ALLOW_TYPE_NUMBER, DEFAULT_UID,
        DEFAULT_BUNDLENAME, 10, "reason", ReasonCodeEnum::REASON_APP_API);
    StandbyServiceImpl::GetInstance()->UpdateRecord(allowRecord, resourceRequest);
    SleepForFC();
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->allowInfoMap_.size(), 0);
}

/**
 * @tc.name: StandbyServiceUnitTest_016
 * @tc.desc: test DayNightSwitchCallback of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_016, TestSize.Level1)
{
    StandbyServiceImpl::GetInstance()->standbyStateManager_->TransitToStateInner(StandbyState::WORKING);
    SleepForFC();
    StandbyServiceImpl::GetInstance()->DayNightSwitchCallback();
    SleepForFC();
    StandbyServiceImpl::GetInstance()->standbyStateManager_->TransitToStateInner(StandbyState::SLEEP);
    SleepForFC();
    StandbyServiceImpl::GetInstance()->DayNightSwitchCallback();
    SleepForFC();
    StandbyServiceImpl::GetInstance()->UnregisterTimeObserver();
    StandbyServiceImpl::GetInstance()->DayNightSwitchCallback();
    StandbyServiceImpl::GetInstance()->RegisterTimeObserver();
    SleepForFC();
    EXPECT_NE(StandbyServiceImpl::GetInstance()->dayNightSwitchTimerId_, 0);
    MockStartTimer(false);
    StandbyServiceImpl::GetInstance()->DayNightSwitchCallback();
    MockStartTimer(true);
    SleepForFC();
}

/**
 * @tc.name: StandbyServiceUnitTest_017
 * @tc.desc: test DayNightSwitchCallback of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_017, TestSize.Level1)
{
    auto allowRecord = std::make_shared<AllowRecord>(DEFAULT_UID, 0, DEFAULT_BUNDLENAME, AllowType::NETWORK);
    allowRecord->allowTimeList_.emplace_back(AllowTime{0, INT64_MAX, "reason"});
    StandbyServiceImpl::GetInstance()->allowInfoMap_.emplace(DEFAULT_KEY, allowRecord);

    std::vector<AllowInfo> allowInfoList;
    StandbyServiceImpl::GetInstance()->GetAllowList(MAX_ALLOW_TYPE_NUMBER, allowInfoList,
        ReasonCodeEnum::REASON_APP_API);
    StandbyServiceImpl::GetInstance()->GetAllowList(MAX_ALLOW_TYPE_NUMBER, allowInfoList,
        ReasonCodeEnum::REASON_NATIVE_API);
    StandbyServiceImpl::GetInstance()->GetAllowListInner(MAX_ALLOW_TYPE_NUMBER, allowInfoList,
        ReasonCodeEnum::REASON_APP_API);
    StandbyServiceImpl::GetInstance()->GetAllowListInner(MAX_ALLOW_TYPE_NUMBER, allowInfoList,
        ReasonCodeEnum::REASON_NATIVE_API);
    StandbyServiceImpl::GetInstance()->GetTemporaryAllowList(MAX_ALLOW_TYPE_NUMBER, allowInfoList,
        ReasonCodeEnum::REASON_APP_API);
    StandbyServiceImpl::GetInstance()->GetTemporaryAllowList(MAX_ALLOW_TYPE_NUMBER, allowInfoList,
        ReasonCodeEnum::REASON_NATIVE_API);
    StandbyServiceImpl::GetInstance()->GetPersistAllowList(MAX_ALLOW_TYPE_NUMBER, allowInfoList,
        true, true);
    StandbyServiceImpl::GetInstance()->GetPersistAllowList(MAX_ALLOW_TYPE_NUMBER, allowInfoList,
        true, false);
    sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest();
    StandbyServiceImpl::GetInstance()->UnapplyAllowResource(resourceRequest);
    StandbyServiceImpl::GetInstance()->DayNightSwitchCallback();
    SleepForFC();
    StandbyServiceImpl::GetInstance()->UnapplyAllowResInner(DEFAULT_UID, DEFAULT_BUNDLENAME, 1, false);
    StandbyServiceImpl::GetInstance()->UnapplyAllowResInner(DEFAULT_UID, DEFAULT_BUNDLENAME, 1, true);
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->allowInfoMap_.size(), 0);
}

/**
 * @tc.name: StandbyServiceUnitTest_018
 * @tc.desc: test AddSubscriber of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_018, TestSize.Level1)
{
    std::vector<std::string> argsInStr {};
    std::string result {};
    StandbyStateSubscriber::GetInstance()->ShellDump(argsInStr, result);
    StandbyStateSubscriber::GetInstance()->NotifyIdleModeByCallback(false, false);
    StandbyStateSubscriber::GetInstance()->ReportAllowListChanged(DEFAULT_UID, DEFAULT_BUNDLENAME,
        AllowType::NETWORK, true);
    sptr<IStandbyServiceSubscriber> nullSubscriber = nullptr;
    EXPECT_NE(StandbyStateSubscriber::GetInstance()->AddSubscriber(nullSubscriber), ERR_OK);
    EXPECT_NE(StandbyStateSubscriber::GetInstance()->RemoveSubscriber(nullSubscriber), ERR_OK);
    sptr<IStandbyServiceSubscriber> subscriber = new (std::nothrow) StandbyServiceSubscriberStub();
    EXPECT_EQ(StandbyStateSubscriber::GetInstance()->AddSubscriber(subscriber), ERR_OK);
    StandbyStateSubscriber::GetInstance()->ShellDump(argsInStr, result);
    EXPECT_NE(StandbyStateSubscriber::GetInstance()->AddSubscriber(subscriber), ERR_OK);
    StandbyStateSubscriber::GetInstance()->NotifyIdleModeByCallback(false, false);
    StandbyStateSubscriber::GetInstance()->ReportAllowListChanged(DEFAULT_UID, DEFAULT_BUNDLENAME,
        AllowType::NETWORK, true);
    EXPECT_EQ(StandbyStateSubscriber::GetInstance()->RemoveSubscriber(subscriber), ERR_OK);
    StandbyStateSubscriber::GetInstance()->RemoveSubscriber(subscriber);
    StandbyStateSubscriber::GetInstance()->AddSubscriber(subscriber);
    auto remote = subscriber->AsObject();
    StandbyStateSubscriber::GetInstance()->HandleSubscriberDeath(remote);
    StandbyStateSubscriber::GetInstance()->HandleSubscriberDeath(remote);
    StandbyStateSubscriber::GetInstance()->RemoveSubscriber(subscriber);

    StandbyStateSubscriber::GetInstance()->deathRecipient_ = nullptr;
    EXPECT_NE(StandbyStateSubscriber::GetInstance()->AddSubscriber(subscriber), ERR_OK);
    EXPECT_NE(StandbyStateSubscriber::GetInstance()->RemoveSubscriber(subscriber), ERR_OK);
    StandbyStateSubscriber::GetInstance()->deathRecipient_ = new (std::nothrow) SubscriberDeathRecipient();

    remote = nullptr;
    StandbyStateSubscriber::GetInstance()->HandleSubscriberDeath(remote);
    sptr<IRemoteObject> proxy {nullptr};
    remote = proxy;
    StandbyStateSubscriber::GetInstance()->HandleSubscriberDeath(remote);
    StandbyStateSubscriber::GetInstance()->HandleSubscriberDeath(remote);
}

/**
 * @tc.name: StandbyServiceUnitTest_019
 * @tc.desc: test AddSubscriber of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_019, TestSize.Level1)
{
    auto allowRecord = std::make_shared<AllowRecord>(DEFAULT_UID, 0, DEFAULT_BUNDLENAME, AllowType::NETWORK);
    auto value = allowRecord->ParseToJson();
    allowRecord->ParseFromJson(value);
    allowRecord->allowTimeList_.emplace_back(AllowTime{0, 0, "reason"});
    value = allowRecord->ParseToJson();
    nlohmann::json emptyValue {};
    allowRecord->ParseFromJson(emptyValue);
    allowRecord->ParseFromJson(value);
    EXPECT_FALSE(allowRecord->allowTimeList_.empty());
}

/**
 * @tc.name: StandbyServiceUnitTest_020
 * @tc.desc: test TimeProvider of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_020, TestSize.Level1)
{
    int64_t curSecTimeStamp = MiscServices::TimeServiceClient::GetInstance()->
        GetWallTimeMs() / TimeConstant::MSEC_PER_SEC;
    struct tm curLocalTime {};
    EXPECT_TRUE(TimeProvider::ConvertTimeStampToLocalTime(curSecTimeStamp, curLocalTime));
    TimeProvider::GetCondition();
    int64_t timeDiff {0};
    TimeProvider::TimeDiffToDayNightSwitch(timeDiff);
    TimeProvider::DiffToFixedClock(0, 0, 0, timeDiff);
    TimeProvider::GetNapTimeOut();
}

/**
 * @tc.name: StandbyServiceUnitTest_021
 * @tc.desc: test TimedTask of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_021, TestSize.Level1)
{
    auto timedTask = std::make_shared<TimedTask>();
    timedTask->OnTrigger();
    auto callBack = [](){};
    timedTask->SetCallbackInfo(callBack);
    timedTask->OnTrigger();
    EXPECT_NE(timedTask, nullptr);
}

/**
 * @tc.name: StandbyServiceUnitTest_022
 * @tc.desc: test observer of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_022, TestSize.Level1)
{
    std::list<SystemProcessInfo> systemProcessInfos {};
    AbilityManagerHelper::GetInstance()->GetRunningSystemProcess(systemProcessInfos);
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos {};
    AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos);
    AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos);
    AppMgrHelper::GetInstance()->appMgrProxy_ = nullptr;
    AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos);

    BundleManagerHelper::GetInstance()->GetClientBundleName(0);
    BundleManagerHelper::GetInstance()->bundleMgr_ = nullptr;
    BundleManagerHelper::GetInstance()->GetClientBundleName(0);
    AppExecFwk::ApplicationInfo applicationInfo {};
    BundleManagerHelper::GetInstance()->GetApplicationInfo(DEFAULT_BUNDLENAME,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, 0, applicationInfo);
    EXPECT_EQ(BundleManagerHelper::GetInstance()->bundleMgr_, nullptr);

    EXPECT_NE(StandbyServiceImpl::GetInstance()->commonEventObserver_, nullptr);
    EventFwk::CommonEventData eventData = EventFwk::CommonEventData();
    StandbyServiceImpl::GetInstance()->commonEventObserver_->OnReceiveEvent(eventData);
    for (const auto& eventName : COMMON_EVENT_LIST) {
        AAFwk::Want want = AAFwk::Want();
        want.SetAction(eventName);
        eventData.SetWant(want);
        StandbyServiceImpl::GetInstance()->commonEventObserver_->OnReceiveEvent(eventData);
    }
    SleepForFC();
}

/**
 * @tc.name: StandbyServiceUnitTest_023
 * @tc.desc: test GetPidAndProcName of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_023, TestSize.Level1)
{
    std::unordered_map<int32_t, std::string> pidNameMap {};
    StandbyServiceImpl::GetInstance()->GetPidAndProcName(pidNameMap);
    EXPECT_NE(pidNameMap.size(), 0);
    MockGetAllRunningProcesses(false);
    StandbyServiceImpl::GetInstance()->GetPidAndProcName(pidNameMap);
    MockGetAllRunningProcesses(true);
    MockGetRunningSystemProcess(false);
    StandbyServiceImpl::GetInstance()->GetPidAndProcName(pidNameMap);
    MockGetRunningSystemProcess(true);
}

/**
 * @tc.name: StandbyServiceUnitTest_024
 * @tc.desc: test multithread OnReceiveEvent of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_024, TestSize.Level1, 20)
{
    EventFwk::CommonEventData eventData = EventFwk::CommonEventData();
    StandbyServiceImpl::GetInstance()->commonEventObserver_->OnReceiveEvent(eventData);
    for (const auto& event : COMMON_EVENT_LIST) {
        AAFwk::Want want = AAFwk::Want();
        want.SetAction(event);
        eventData.SetWant(want);
        StandbyServiceImpl::GetInstance()->commonEventObserver_->OnReceiveEvent(eventData);
    }
    StandbyServiceUnitTest::SleepForFC();
    EXPECT_NE(StandbyServiceImpl::GetInstance()->commonEventObserver_, nullptr);
    MockSubscribeCommonEvent(false);
    StandbyServiceImpl::GetInstance()->commonEventObserver_->Subscribe();
    EXPECT_TRUE(StandbyServiceImpl::GetInstance()->commonEventObserver_->Unsubscribe());
    MockSubscribeCommonEvent(true);
    StandbyServiceImpl::GetInstance()->commonEventObserver_->Subscribe();
    EXPECT_TRUE(StandbyServiceImpl::GetInstance()->commonEventObserver_->Unsubscribe());
}

/**
 * @tc.name: StandbyServiceUnitTest_025
 * @tc.desc: test multithread init of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_025, TestSize.Level1, 20)
{
    StandbyServiceImpl::GetInstance()->InitReadyState();
    EXPECT_TRUE(StandbyServiceImpl::GetInstance()->isServiceReady_.load());
    StandbyServiceUnitTest::SleepForFC();
}

/**
 * @tc.name: StandbyServiceUnitTest_026
 * @tc.desc: test multithread uninit of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_026, TestSize.Level1, 20)
{
    StandbyServiceImpl::GetInstance()->UninitReadyState();
    EXPECT_FALSE(StandbyServiceImpl::GetInstance()->isServiceReady_.load());
    StandbyServiceUnitTest::SleepForFC();
}

/**
 * @tc.name: StandbyServiceUnitTest_027
 * @tc.desc: test multithread DayNightSwitchCallback of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_027, TestSize.Level1, 20)
{
    StandbyServiceImpl::GetInstance()->InitReadyState();
    StandbyServiceImpl::GetInstance()->DayNightSwitchCallback();
    StandbyServiceUnitTest::SleepForFC();
    EXPECT_TRUE(StandbyServiceImpl::GetInstance()->isServiceReady_.load());
}

/**
 * @tc.name: StandbyServiceUnitTest_028
 * @tc.desc: test multithread ApplyAllowResource of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_028, TestSize.Level1, 20)
{
    sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest();
    StandbyServiceImpl::GetInstance()->ApplyAllowResource(resourceRequest);
    sptr<ResourceRequest> validResRequest = new (std::nothrow) ResourceRequest(AllowType::NETWORK,
        0, "test_process", 100, "test", 1);
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->ApplyAllowResource(validResRequest), ERR_OK);
    StandbyServiceUnitTest::SleepForFC();
}

/**
 * @tc.name: StandbyServiceUnitTest_029
 * @tc.desc: test multithread UnapplyAllowResource of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_029, TestSize.Level1, 20)
{
    sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest();
    StandbyServiceImpl::GetInstance()->UnapplyAllowResource(resourceRequest);
    sptr<ResourceRequest> validResRequest = new (std::nothrow) ResourceRequest(AllowType::NETWORK,
        0, "test_process", 100, "test", 1);
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->UnapplyAllowResource(validResRequest), ERR_OK);
}

/**
 * @tc.name: StandbyServiceUnitTest_030
 * @tc.desc: test multithread GetAllowList of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_030, TestSize.Level1, 20)
{
    std::vector<AllowInfo> allowInfoList;
    StandbyServiceImpl::GetInstance()->GetAllowList(AllowType::NETWORK, allowInfoList, 0);
    EXPECT_EQ(allowInfoList.size(), 0);
}

/**
 * @tc.name: StandbyServiceUnitTest_031
 * @tc.desc: test multithread IsDeviceInStandby of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_031, TestSize.Level1, 20)
{
    bool isStandby {false};
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->IsDeviceInStandby(isStandby), ERR_OK);
}

/**
 * @tc.name: StandbyServiceUnitTest_032
 * @tc.desc: test multithread ShellDump of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_032, TestSize.Level1, 20)
{
    std::vector<std::string> argsInStr {};
    std::string result;
    StandbyServiceImpl::GetInstance()->ShellDump(argsInStr, result);
    EXPECT_NE(result.size(), 0);
}

/**
 * @tc.name: StandbyServiceUnitTest_033
 * @tc.desc: test RegisterCommEventObserver of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_033, TestSize.Level1, 20)
{
    StandbyServiceImpl::GetInstance()->RegisterCommEventObserver();
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->UnregisterCommEventObserver(), ERR_OK);
    StandbyServiceImpl::GetInstance()->RegisterCommEventObserver();
    StandbyServiceImpl::GetInstance()->RegisterCommEventObserver();
    MockSubscribeCommonEvent(false);
    StandbyServiceImpl::GetInstance()->RegisterCommEventObserver();
    StandbyServiceImpl::GetInstance()->UnregisterCommEventObserver();
    MockSubscribeCommonEvent(true);
}

/**
 * @tc.name: StandbyServiceUnitTest_034
 * @tc.desc: test RegisterCommEventObserver of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_034, TestSize.Level1, 20)
{
    StandbyServiceImpl::GetInstance()->RegisterTimeObserver();
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->UnregisterTimeObserver(), ERR_OK);
    StandbyServiceImpl::GetInstance()->ResetTimeObserver();
    StandbyServiceImpl::GetInstance()->dayNightSwitchTimerId_ = 1;
    StandbyServiceImpl::GetInstance()->RegisterTimeObserver();
    StandbyServiceImpl::GetInstance()->dayNightSwitchTimerId_ = 0;
    MockSubscribeCommonEvent(false);
    StandbyServiceImpl::GetInstance()->RegisterTimeObserver();
    MockSubscribeCommonEvent(true);
}

/**
 * @tc.name: StandbyServiceUnitTest_035
 * @tc.desc: test AddSubscriber of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_035, TestSize.Level1, 20)
{
    sptr<IStandbyServiceSubscriber> subscriber = new (std::nothrow) StandbyServiceSubscriberStub();
    StandbyStateSubscriber::GetInstance()->AddSubscriber(subscriber);
    StandbyStateSubscriber::GetInstance()->RemoveSubscriber(subscriber);
    MockPublishCommonEvent(true);
    StandbyStateSubscriber::GetInstance()->ReportStandbyState(StandbyState::WORKING);
    StandbyStateSubscriber::GetInstance()->ReportStandbyState(StandbyState::NAP);
    StandbyStateSubscriber::GetInstance()->ReportStandbyState(StandbyState::SLEEP);
    MockPublishCommonEvent(false);
    StandbyStateSubscriber::GetInstance()->ReportStandbyState(StandbyState::WORKING);
    StandbyStateSubscriber::GetInstance()->ReportStandbyState(StandbyState::NAP);
    StandbyStateSubscriber::GetInstance()->ReportStandbyState(StandbyState::SLEEP);
    MockPublishCommonEvent(true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: StandbyServiceUnitTest_036
 * @tc.desc: test OnRemoteRequestInner of StandbyStateSubscriber.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_036, TestSize.Level1, 20)
{
    sptr<StandbyServiceSubscriberStub> subscriber = new (std::nothrow) StandbyServiceSubscriberStub();
    MessageParcel data {};
    MessageParcel reply {};
    MessageOption option {MessageOption::TF_ASYNC};
    data.WriteInterfaceToken(IStandbyServiceSubscriber::GetDescriptor());
    subscriber->OnRemoteRequest(
        (static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_DEVICE_IDLE_MODE)), data, reply, option);
    subscriber->OnRemoteRequest(
        (static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_ALLOW_LIST_CHANGED)), data, reply, option);
    auto ret = subscriber->OnRemoteRequest(
        static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_ALLOW_LIST_CHANGED) + 1, data, reply, option);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: StandbyServiceUnitTest_037
 * @tc.desc: test OnRemoteRequestInner of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_037, TestSize.Level1, 20)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    data.WriteInterfaceToken(IStandbyService::GetDescriptor());
    StandbyService::GetInstance()->OnRemoteRequest(
        (static_cast<uint32_t>(IStandbyInterfaceCode::SUBSCRIBE_STANDBY_CALLBACK)), data, reply, option);
    StandbyService::GetInstance()->OnRemoteRequest(
        (static_cast<uint32_t>(IStandbyInterfaceCode::UNSUBSCRIBE_STANDBY_CALLBACK)),
        data, reply, option);
    StandbyService::GetInstance()->OnRemoteRequest(
        (static_cast<uint32_t>(IStandbyInterfaceCode::APPLY_ALLOW_RESOURCE)), data, reply, option);
    StandbyService::GetInstance()->OnRemoteRequest(
        (static_cast<uint32_t>(IStandbyInterfaceCode::UNAPPLY_ALLOW_RESOURCE)), data, reply, option);
    StandbyService::GetInstance()->OnRemoteRequest(
        (static_cast<uint32_t>(IStandbyInterfaceCode::GET_ALLOW_LIST)), data, reply, option);
    StandbyService::GetInstance()->OnRemoteRequest(
        (static_cast<uint32_t>(IStandbyInterfaceCode::IS_DEVICE_IN_STANDBY)), data, reply, option);
    auto ret = StandbyService::GetInstance()->OnRemoteRequest(
        (static_cast<uint32_t>(IStandbyInterfaceCode::IS_DEVICE_IN_STANDBY)) + 1, data, reply, option);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: StandbyServiceUnitTest_038
 * @tc.desc: test TimedTask.
 * @tc.type: FUNC
 * @tc.require: AR000HQ76V
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_038, TestSize.Level1)
{
    auto timedTask = std::make_shared<TimedTask>(false, 0, true);
    timedTask = std::make_shared<TimedTask>(false, 0, false);
    uint64_t timerId {0};
    timedTask->StartDayNightSwitchTimer(timerId);
    std::function<void()> callBack {};
    MockStartTimer(false);
    uint64_t zeroTimeId {0};
    uint64_t negativeTimeId {-1};
    EXPECT_TRUE(timedTask->RegisterDayNightSwitchTimer(zeroTimeId, false, 0, callBack));
    EXPECT_TRUE(timedTask->RegisterDayNightSwitchTimer(negativeTimeId, false, 0, callBack));
    timedTask->StartDayNightSwitchTimer(timerId);
    TimedTask::CreateTimer(false, 0, false, false, callBack);
    StandbyServiceImpl::GetInstance()->UnregisterTimeObserver();
    StandbyServiceImpl::GetInstance()->DayNightSwitchCallback();
    StandbyServiceImpl::GetInstance()->RegisterTimeObserver();
    StandbyServiceUnitTest::SleepForFC();
    StandbyServiceImpl::GetInstance()->ResetTimeObserver();
    StandbyServiceUnitTest::SleepForFC();
    MockStartTimer(true);
    timedTask->RegisterDayNightSwitchTimer(zeroTimeId, false, 0, callBack);
    timedTask->RegisterDayNightSwitchTimer(negativeTimeId, false, 0, callBack);
}

/**
 * @tc.name: StandbyServiceUnitTest_039
 * @tc.desc: test CheckNativePermission of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyServiceUnitTest, StandbyServiceUnitTest_039, TestSize.Level1)
{
    Security::AccessToken::AccessTokenID tokenId {};
    EXPECT_EQ(StandbyServiceImpl::GetInstance()->CheckNativePermission(tokenId), ERR_OK);
    MockGetTokenTypeFlag(false);
    StandbyServiceImpl::GetInstance()->CheckNativePermission(tokenId);
    MockGetTokenTypeFlag(true);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
