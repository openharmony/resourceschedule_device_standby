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

#include "gtest/gtest.h"
#include "gtest/hwext/gtest-multithread.h"
#include "singleton.h"

#include "common_event_support.h"
#include "common_event_observer.h"
#include "allow_type.h"
#include "standby_service_client.h"
#include "standby_service.h"
#include "standby_service_impl.h"
#include "standby_state_subscriber.h"
#include "standby_state_subscriber.h"
#include "standby_service_subscriber_stub.h"
#include "device_standby_switch.h"

#include "state_manager_adapter.h"
#include "constraint_manager_adapter.h"
#include "listener_manager_adapter.h"
#include "strategy_manager_adapter.h"
#include "standby_config_manager.h"
#include "common_event_listener.h"
#include "charge_state_monitor.h"
#include "motion_sensor_monitor.h"

using namespace testing::ext;
using namespace testing::mt;


namespace OHOS {
namespace DevStandbyMgr {
namespace {
    const vector<std::string> COMMON_EVENT_LIST = {
        EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON,
        EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF,
        EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING,
        EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED,
        EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING,
        EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED,
        EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON,
        EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED,
    };
    constexpr int32_t SLEEP_TIMEOUT = 500;
}

class StandbyPluginUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override {}
    void TearDown() override {}

    inline static void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMEOUT));
    }
private:
    static std::shared_ptr<ConstraintManagerAdapter> constraintManager_;
    static std::shared_ptr<ListenerManagerAdapter> listenerManager_;
    static std::shared_ptr<StrategyManagerAdapter> strategyManager_;
    static std::shared_ptr<StateManagerAdapter> standbyStateManager_;
};

void StandbyPluginUnitTest::TearDownTestCase()
{
    SleepForFC();
    StandbyServiceImpl::GetInstance()->UnInit();
    if (StandbyServiceImpl::GetInstance()->handler_) {
        StandbyServiceImpl::GetInstance()->handler_->RemoveAllEvents();
        auto runner = StandbyServiceImpl::GetInstance()->handler_->GetEventRunner();
        if (runner) {
            runner->Stop();
            runner = nullptr;
        }
        StandbyServiceImpl::GetInstance()->handler_ = nullptr;
    }
}

std::shared_ptr<ConstraintManagerAdapter> StandbyPluginUnitTest::constraintManager_ {nullptr};
std::shared_ptr<ListenerManagerAdapter> StandbyPluginUnitTest::listenerManager_ {nullptr};
std::shared_ptr<StrategyManagerAdapter> StandbyPluginUnitTest::strategyManager_ {nullptr};
std::shared_ptr<StateManagerAdapter> StandbyPluginUnitTest::standbyStateManager_ {nullptr};

void StandbyPluginUnitTest::SetUpTestCase()
{
    StandbyServiceImpl::GetInstance()->Init();
    SleepForFC();

    constraintManager_ = std::make_shared<ConstraintManagerAdapter>();
    listenerManager_ = std::make_shared<ListenerManagerAdapter>();
    strategyManager_ = std::make_shared<StrategyManagerAdapter>();
    standbyStateManager_ = std::make_shared<StateManagerAdapter>();

    StandbyServiceImpl::GetInstance()->constraintManager_ = constraintManager_;
    StandbyServiceImpl::GetInstance()->listenerManager_ = listenerManager_;
    StandbyServiceImpl::GetInstance()->strategyManager_ = strategyManager_;
    StandbyServiceImpl::GetInstance()->standbyStateManager_ = standbyStateManager_;
    StandbyServiceImpl::GetInstance()->InitReadyState();
    SleepForFC();
}

/**
 * @tc.name: StandbyPluginUnitTest_001
 * @tc.desc: test Init of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_001, TestSize.Level1)
{
    listenerManager_->StopListener();
    listenerManager_->UnInit();
    constraintManager_->UnInit();
    strategyManager_->strategyList_.clear();
    standbyStateManager_->UnInit();
    standbyStateManager_->Init();
    strategyManager_->Init();
    constraintManager_->Init();
    listenerManager_->Init();
    listenerManager_->StartListener();
    EXPECT_NE(listenerManager_, nullptr);
}

/**
 * @tc.name: StandbyPluginUnitTest_002
 * @tc.desc: test RegisterPolicy of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_002, TestSize.Level1)
{
    strategyManager_->RegisterPolicy({"NETWORK", "TIMER", "RUNNING_LOCK", "WORK_SCHEDULER", ""});
    EXPECT_NE(strategyManager_->strategyList_.size(), 0);
}

/**
 * @tc.name: StandbyPluginUnitTest_003
 * @tc.desc: test HandleEvent of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_003, TestSize.Level1)
{
    StandbyMessage message(StandbyMessageType::PHASE_TRANSIT);
    standbyStateManager_->HandleEvent(message);
    strategyManager_->HandleEvent(message);
    StandbyMessage commonEventMessage(StandbyMessageType::COMMON_EVENT);
    standbyStateManager_->HandleEvent(commonEventMessage);
    strategyManager_->HandleEvent(commonEventMessage);
    StandbyMessage conditionChangeMessage(StandbyMessageType::RES_CTRL_CONDITION_CHANGED);
    standbyStateManager_->HandleEvent(conditionChangeMessage);
    strategyManager_->HandleEvent(conditionChangeMessage);
    EXPECT_NE(listenerManager_, nullptr);
}

/**
 * @tc.name: StandbyPluginUnitTest_004
 * @tc.desc: test CommonEventListener of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_004, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo {};
    auto commonEventListener = std::make_shared<CommonEventListener>(subscribeInfo);

    EventFwk::CommonEventData eventData = EventFwk::CommonEventData();
    commonEventListener->OnReceiveEvent(eventData);
    for (const auto& eventName : COMMON_EVENT_LIST) {
        AAFwk::Want want = AAFwk::Want();
        want.SetAction(eventName);
        eventData.SetWant(want);
        commonEventListener->OnReceiveEvent(eventData);
    }
    SleepForFC();
    SleepForFC();
    for (const auto& eventName : COMMON_EVENT_LIST) {
        StandbyMessage message;
        message.action_ = eventName;
        standbyStateManager_->HandleCommonEvent(message);
    }
    standbyStateManager_->TransitToState(StandbyState::WORKING);
    SleepForFC();
    EXPECT_NE(commonEventListener, nullptr);
}

/**
 * @tc.name: StandbyPluginUnitTest_005
 * @tc.desc: test ChargeStateMonitor of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_005, TestSize.Level1)
{
    auto chargeStateMonitor = std::make_shared<ChargeStateMonitor>();
    chargeStateMonitor->Init();
    chargeStateMonitor->StartMonitoring();
    EXPECT_NE(chargeStateMonitor, nullptr);
}

/**
 * @tc.name: StandbyPluginUnitTest_006
 * @tc.desc: test ChargeStateMonitor of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_006, TestSize.Level1)
{
    StandbyConfigManager::GetInstance()->standbySwitchMap_[DETECT_MOTION_CONFIG] = false;
    constraintManager_->UnInit();
    constraintManager_->Init();
    StandbyConfigManager::GetInstance()->standbySwitchMap_[DETECT_MOTION_CONFIG] = true;
    constraintManager_->UnInit();
    constraintManager_->Init();
    constraintManager_->isEvaluation_ = true;
    constraintManager_->StopEvalution();
    constraintManager_->isEvaluation_ = false;
    constraintManager_->curMonitor_ = nullptr;
    constraintManager_->StopEvalution();
    constraintManager_->isEvaluation_ = true;
    ConstraintEvalParam params;
    constraintManager_->StartEvalution(params);
    constraintManager_->isEvaluation_ = false;
    constraintManager_->StartEvalution(params);
    std::shared_ptr<ChargeStateMonitor> monitor = nullptr;
    constraintManager_->RegisterConstraintCallback(params, monitor);
    constraintManager_->StartEvalution(params);
    constraintManager_->constraintMap_.erase(params.GetHashValue());
    ConstraintEvalParam repeatedMotionParams{StandbyState::SLEEP, SleepStatePhase::END, StandbyState::SLEEP,
            SleepStatePhase::END};
    repeatedMotionParams.isRepeatedDetection_ = true;
    constraintManager_->StartEvalution(repeatedMotionParams);
    SleepForFC();
    ConstraintEvalParam motionDetectParams{StandbyState::NAP, NapStatePhase::END, StandbyState::SLEEP,
            SleepStatePhase::SYS_RES_DEEP};
    constraintManager_->StartEvalution(motionDetectParams);
    constraintManager_->StopEvalution();
    standbyStateManager_->ExitStandby(StandbyState::WORKING);
    SleepForFC();
    EXPECT_FALSE(constraintManager_->isEvaluation_);
}

/**
 * @tc.name: StandbyPluginUnitTest_007
 * @tc.desc: test MotionSensorMonitor of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_007, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->StopMonitoring();
    repeatedMotionConstraint->isMonitoring_ = true;
    repeatedMotionConstraint->StopMonitoring();
    repeatedMotionConstraint->isMonitoring_ = false;
    repeatedMotionConstraint->StopMonitoring();
    EXPECT_FALSE(repeatedMotionConstraint->isMonitoring_);
}

/**
 * @tc.name: StandbyPluginUnitTest_008
 * @tc.desc: test CheckTransitionValid of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_008, TestSize.Level1)
{
    for (auto &statePtr : standbyStateManager_->indexToState_) {
        for (uint32_t nextState = StandbyState::WORKING; nextState <= StandbyState::SLEEP; ++nextState) {
            statePtr->CheckTransitionValid(nextState);
        }
    }
    EXPECT_TRUE(standbyStateManager_->CheckTransitionValid(StandbyState::WORKING, StandbyState::WORKING));
}

/**
 * @tc.name: StandbyPluginUnitTest_009
 * @tc.desc: test TransitToState of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_009, TestSize.Level1)
{
    standbyStateManager_->isEvalution_=true;
    standbyStateManager_->ExitStandby(StandbyState::WORKING);
    standbyStateManager_->isEvalution_=false;
    standbyStateManager_->ExitStandby(StandbyState::WORKING);
    standbyStateManager_->TransitToState(standbyStateManager_->curStatePtr_->GetCurState());
    standbyStateManager_->TransitToState(StandbyState::MAINTENANCE);
    standbyStateManager_->TransitToState(StandbyState::NAP);
    SleepForFC();
    standbyStateManager_->TransitToState(StandbyState::MAINTENANCE);
    standbyStateManager_->TransitToState(StandbyState::NAP);
    standbyStateManager_->TransitToState(StandbyState::WORKING);
    standbyStateManager_->SendNotification(StandbyState::WORKING, true);
    standbyStateManager_->SendNotification(StandbyState::WORKING, false);

    standbyStateManager_->isBlocked_ = true;
    standbyStateManager_->EnterStandby(StandbyState::WORKING);
    standbyStateManager_->isBlocked_ = false;
    standbyStateManager_->EnterStandby(StandbyState::WORKING);
    standbyStateManager_->TransitToState(StandbyState::WORKING);
    EXPECT_NE(standbyStateManager_->GetCurState(), StandbyState::NAP);
}

/**
 * @tc.name: StandbyPluginUnitTest_010
 * @tc.desc: test NapState of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_010, TestSize.Level1)
{
    standbyStateManager_->darkStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->darkStatePtr_->EndEvalCurrentState(true);

    standbyStateManager_->darkStatePtr_->curPhase_ = NapStatePhase::CONNECTION;
    standbyStateManager_->napStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->napStatePtr_->EndEvalCurrentState(true);
    SleepForFC();
    standbyStateManager_->darkStatePtr_->curPhase_ = NapStatePhase::SYS_RES_LIGHT;
    standbyStateManager_->napStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->napStatePtr_->EndEvalCurrentState(true);
    SleepForFC();
    standbyStateManager_->napStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->napStatePtr_->EndEvalCurrentState(true);
    SleepForFC();
    EXPECT_NE(standbyStateManager_->curStatePtr_->GetCurState(), StandbyState::MAINTENANCE);
}

/**
 * @tc.name: StandbyPluginUnitTest_011
 * @tc.desc: test SleepState of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_011, TestSize.Level1)
{
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(true);

    standbyStateManager_->sleepStatePtr_->curPhase_ = SleepStatePhase::SYS_RES_DEEP;
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(true);
    SleepForFC();
    standbyStateManager_->sleepStatePtr_->curPhase_ = SleepStatePhase::APP_RES_HARDWARE;
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(true);
    SleepForFC();
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(true);
    SleepForFC();
    EXPECT_NE(standbyStateManager_->curStatePtr_->GetCurState(), StandbyState::WORKING);
}

/**
 * @tc.name: StandbyPluginUnitTest_012
 * @tc.desc: test TransitToStateInner of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_012, TestSize.Level1)
{
    standbyStateManager_->TransitToStateInner(StandbyState::WORKING);
    standbyStateManager_->TransitToStateInner(StandbyState::DARK);
    standbyStateManager_->TransitToStateInner(StandbyState::NAP);
    SleepForFC();
    standbyStateManager_->TransitToStateInner(StandbyState::MAINTENANCE);
    standbyStateManager_->TransitToStateInner(StandbyState::NAP);
    standbyStateManager_->TransitToStateInner(StandbyState::SLEEP);
    SleepForFC();
    standbyStateManager_->TransitToStateInner(StandbyState::MAINTENANCE);
    standbyStateManager_->TransitToStateInner(StandbyState::SLEEP);
    EXPECT_NE(standbyStateManager_->curStatePtr_->GetCurState(), StandbyState::WORKING);
}

/**
 * @tc.name: StandbyPluginUnitTest_013
 * @tc.desc: test multithread OnReceiveEvent of StandbyService.
 * @tc.type: FUNC
 * @tc.require: AR000HQ6GA
 */
HWMTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_013, TestSize.Level1, 20)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo {};
    auto commonEventListener = std::make_shared<CommonEventListener>(subscribeInfo);

    EventFwk::CommonEventData eventData = EventFwk::CommonEventData();
    commonEventListener->OnReceiveEvent(eventData);
    for (const auto& event : COMMON_EVENT_LIST) {
        AAFwk::Want want = AAFwk::Want();
        want.SetAction(event);
        eventData.SetWant(want);
        commonEventListener->OnReceiveEvent(eventData);
    }
    EXPECT_TRUE(true);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
