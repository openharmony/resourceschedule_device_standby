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
#define private public
#define protected public

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
#include "charge_state_monitor.h"
#ifdef STANDBY_SENSORS_SENSOR_ENABLE
#include "motion_sensor_monitor.h"
#endif
#ifdef STANDBY_MULTIMODALINPUT_INPUT_ENABLE
#include "input_manager.h"
#endif
#ifdef ENABLE_BACKGROUND_TASK_MGR
#include "background_task_listener.h"
#endif
#include "input_manager_listener.h"
#include "common_constant.h"
#include "dark_state.h"

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
 * @tc.require:
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
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_002, TestSize.Level1)
{
    strategyManager_->RegisterPolicy({"NETWORK", "TIMER", "RUNNING_LOCK", "WORK_SCHEDULER", ""});
    EXPECT_FALSE(strategyManager_->strategyList_.empty());
}

/**
 * @tc.name: StandbyPluginUnitTest_003
 * @tc.desc: test HandleEvent of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require:
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
 * @tc.desc: test TransitToState of StandbyStateManager.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_004, TestSize.Level1)
{
    for (const auto& eventName : COMMON_EVENT_LIST) {
        StandbyMessage message;
        message.action_ = eventName;
        standbyStateManager_->HandleCommonEvent(message);
    }
    standbyStateManager_->TransitToState(StandbyState::WORKING);
    SleepForFC();
    EXPECT_NE(standbyStateManager_, nullptr);
}

/**
 * @tc.name: StandbyPluginUnitTest_005
 * @tc.desc: test ChargeStateMonitor of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require:
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
 * @tc.require:
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

#ifdef STANDBY_SENSORS_SENSOR_ENABLE
/**
 * @tc.name: StandbyPluginUnitTest_007
 * @tc.desc: test MotionSensorMonitor of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require:
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
#endif

/**
 * @tc.name: StandbyPluginUnitTest_008
 * @tc.desc: test CheckTransitionValid of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require:
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
 * @tc.require:
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
 * @tc.require:
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

    standbyStateManager_->darkStatePtr_->stateManager_.reset();
    standbyStateManager_->darkStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->darkStatePtr_->stateManager_ = standbyStateManager_;

    standbyStateManager_->maintStatePtr_->stateManager_.reset();
    standbyStateManager_->maintStatePtr_->BeginState();
    standbyStateManager_->maintStatePtr_->stateManager_ = standbyStateManager_;
}

/**
 * @tc.name: StandbyPluginUnitTest_011
 * @tc.desc: test SleepState of StandbyPlugin.
 * @tc.type: FUNC
 * @tc.require:
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
 * @tc.require:
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

#ifdef STANDBY_SENSORS_SENSOR_ENABLE
/**
 * @tc.name: StandbyPluginUnitTest_014
 * @tc.desc: test MotionSensorMonitor of AddEnergy.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0014, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    AccelData* accelData = new AccelData();
    repeatedMotionConstraint->AddEnergy(accelData);
    repeatedMotionConstraint->hasPrevAccelData_ = true;
    repeatedMotionConstraint->AddEnergy(accelData);
    EXPECT_TRUE(repeatedMotionConstraint->hasPrevAccelData_ == true);
}

/**
 * @tc.name: StandbyPluginUnitTest_015
 * @tc.desc: test MotionSensorMonitor of Init.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0015, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->Init();
    repeatedMotionConstraint->params_.isRepeatedDetection_ = true;
    repeatedMotionConstraint->Init();
    repeatedMotionConstraint->params_.isRepeatedDetection_ = false;
    EXPECT_TRUE(repeatedMotionConstraint->Init() == true);
}

/**
 * @tc.name: StandbyPluginUnitTest_016
 * @tc.desc: test MotionSensorMonitor of PeriodlyStartMotionDetection.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0016, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->PeriodlyStartMotionDetection();
    repeatedMotionConstraint->energy_ = 1;
    repeatedMotionConstraint->PeriodlyStartMotionDetection();
    EXPECT_TRUE(repeatedMotionConstraint->StartMonitoringInner() == ERR_OK);
    repeatedMotionConstraint->isMonitoring_ = false;
    repeatedMotionConstraint->StartMonitoringInner();
}

/**
 * @tc.name: StandbyPluginUnitTest_017
 * @tc.desc: test MotionSensorMonitor of StartSensor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0017, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    int32_t sensorTypeId = 1;
    SensorUser sensorUser;
    repeatedMotionConstraint->isMonitoring_ = false;
    repeatedMotionConstraint->StartSensor();
    repeatedMotionConstraint->isMonitoring_ = true;
    repeatedMotionConstraint->StartSensor();
    sensorTypeId = repeatedMotionConstraint->detectionTimeOut_;
    repeatedMotionConstraint->StartSensor();
    EXPECT_TRUE(repeatedMotionConstraint->StartSensor() == ERR_OK);
}

/**
 * @tc.name: StandbyPluginUnitTest_018
 * @tc.desc: test MotionSensorMonitor of StopSensor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0018, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    int32_t sensorTypeId = 1;
    SensorUser sensorUser;
    repeatedMotionConstraint->isMonitoring_ = false;
    repeatedMotionConstraint->StopSensor();
    repeatedMotionConstraint->isMonitoring_ = true;
    repeatedMotionConstraint->StopSensor();
    sensorTypeId = repeatedMotionConstraint->detectionTimeOut_;
    repeatedMotionConstraint->StartSensor();
    EXPECT_TRUE(repeatedMotionConstraint->isMonitoring_ == true);
}
#endif // STANDBY_SENSORS_SENSOR_ENABLE

/**
 * @tc.name: StandbyPluginUnitTest_019
 * @tc.desc: test ConstraintManagerAdapter of Init.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0019, TestSize.Level1)
{
    auto repeatedMotionConstraint = std::make_shared<ConstraintManagerAdapter>();
    repeatedMotionConstraint->Init();
    EXPECT_TRUE(repeatedMotionConstraint->Init());
    StandbyServiceImpl::GetInstance()->GetStateManager();
    repeatedMotionConstraint->Init();
}

/**
 * @tc.name: StandbyPluginUnitTest_020
 * @tc.desc: test ConstraintManagerAdapter of StartEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0020, TestSize.Level1)
{
    auto repeatedMotionConstraint = std::make_shared<ConstraintManagerAdapter>();
    ConstraintEvalParam params;
    repeatedMotionConstraint->StartEvalution(params);
    repeatedMotionConstraint->isEvaluation_ = false;
    repeatedMotionConstraint->StartEvalution(params);
    repeatedMotionConstraint->isEvaluation_ = true;
    repeatedMotionConstraint->UnInit();
    EXPECT_FALSE(repeatedMotionConstraint->StartEvalution(params) == ERR_OK);
}

/**
 * @tc.name: StandbyPluginUnitTest_021
 * @tc.desc: test ConstraintManagerAdapter of StopEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0021, TestSize.Level1)
{
    auto repeatedMotionConstraint = std::make_shared<ConstraintManagerAdapter>();
    repeatedMotionConstraint->StopEvalution();
    repeatedMotionConstraint->isEvaluation_ = false;
    repeatedMotionConstraint->StopEvalution();
    repeatedMotionConstraint->isEvaluation_ = true;
    repeatedMotionConstraint->UnInit();
    EXPECT_FALSE(repeatedMotionConstraint->StopEvalution() == ERR_OK);
}

/**
 * @tc.name: StandbyPluginUnitTest_022
 * @tc.desc: test ChargeStateMonitor of StartMonitoring.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0022, TestSize.Level1)
{
    auto repeatedChargeStateMonitor = std::make_shared<ChargeStateMonitor>();
    repeatedChargeStateMonitor->StartMonitoring();
    standbyStateManager_->napStatePtr_->EndEvalCurrentState(false);
    repeatedChargeStateMonitor->StartMonitoring();
    EXPECT_TRUE(repeatedChargeStateMonitor->Init());
}

/**
 * @tc.name: StandbyPluginUnitTest_023
 * @tc.desc: test StateManagerAdapter of UnInit.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0023, TestSize.Level1)
{
    EXPECT_TRUE(DelayedSingleton<StateManagerAdapter>::GetInstance()->Init());
}

/**
 * @tc.name: StandbyPluginUnitTest_024
 * @tc.desc: test StateManagerAdapter of HandleEvent.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0024, TestSize.Level1)
{
    StandbyMessage message;
    message.eventId_ = StandbyMessageType::COMMON_EVENT;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleEvent(message);
    message.eventId_ = StandbyMessageType::RES_CTRL_CONDITION_CHANGED;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleEvent(message);
    message.eventId_ = 1234;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleEvent(message);
    EXPECT_TRUE(message.eventId_ != StandbyMessageType::COMMON_EVENT);
}

/**
 * @tc.name: StandbyPluginUnitTest_025
 * @tc.desc: test StateManagerAdapter of HandleCommonEvent.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0025, TestSize.Level1)
{
    StandbyMessage message;
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleCommonEvent(message);
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleCommonEvent(message);
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleCommonEvent(message);
    DelayedSingleton<StateManagerAdapter>::GetInstance()->Init();
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleCommonEvent(message);
    message.action_ = "1234";
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleCommonEvent(message);
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleCommonEvent(message);
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleCommonEvent(message);
    EXPECT_TRUE(DelayedSingleton<StateManagerAdapter>::GetInstance()->indexToState_.size() != 0);
}

/**
 * @tc.name: StandbyPluginUnitTest_026
 * @tc.desc: test StateManagerAdapter of HandleScrOffHalfHour.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0026, TestSize.Level1)
{
    StandbyMessage message;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->scrOffHalfHourTimerId_ = 0;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleScrOffHalfHour(message);
    DelayedSingleton<StateManagerAdapter>::GetInstance()->scrOffHalfHourTimerId_ = 1;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleScrOffHalfHour(message);
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleScrOffHalfHour(message);
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleScrOffHalfHour(message);
    EXPECT_TRUE(DelayedSingleton<StateManagerAdapter>::GetInstance()->isScreenOn_);
    message.action_ = "1234";
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleScrOffHalfHour(message);
}

/**
 * @tc.name: StandbyPluginUnitTest_027
 * @tc.desc: test StateManagerAdapter of HandleOpenCloseLid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0027, TestSize.Level1)
{
    StandbyMessage message;
    message.action_ = LID_OPEN;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleOpenCloseLid(message);
    message.action_ = LID_CLOSE;
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleOpenCloseLid(message);
    message.action_ = "1234";
    DelayedSingleton<StateManagerAdapter>::GetInstance()->HandleOpenCloseLid(message);
    EXPECT_TRUE(DelayedSingleton<StateManagerAdapter>::GetInstance()->TransitToState(0) == ERR_OK);
}

/**
 * @tc.name: StandbyPluginUnitTest_028
 * @tc.desc: test StateManagerAdapter of EndEvalCurrentState.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0028, TestSize.Level1)
{
    auto evalResult = true;
    standbyStateManager_->isEvalution_ = false;
    EXPECT_TRUE(standbyStateManager_->EndEvalCurrentState(evalResult) == ERR_STANDBY_STATE_TIMING_SEQ_ERROR);
    standbyStateManager_->isEvalution_ = true;
    standbyStateManager_->EndEvalCurrentState(evalResult);
}

/**
 * @tc.name: StandbyPluginUnitTest_029
 * @tc.desc: test StateManagerAdapter of StopEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0029, TestSize.Level1)
{
    standbyStateManager_->isEvalution_ = false;
    standbyStateManager_->StopEvalution();
    standbyStateManager_->isEvalution_ = true;
    standbyStateManager_->StopEvalution();
    EXPECT_TRUE(standbyStateManager_->isEvalution_ == false);
}

/**
 * @tc.name: StandbyPluginUnitTest_030
 * @tc.desc: test ListenerManagerAdapter of StartListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0030, TestSize.Level1)
{
    DelayedSingleton<ListenerManagerAdapter>::GetInstance()->StartListener();
    DelayedSingleton<ListenerManagerAdapter>::GetInstance()->messageListenerList_.clear();
    EXPECT_TRUE(DelayedSingleton<ListenerManagerAdapter>::GetInstance()->StartListener() == ERR_OK);
}

/**
 * @tc.name: StandbyPluginUnitTest_031
 * @tc.desc: test ListenerManagerAdapter of StopListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0031, TestSize.Level1)
{
    DelayedSingleton<ListenerManagerAdapter>::GetInstance()->StopListener();
    DelayedSingleton<ListenerManagerAdapter>::GetInstance()->messageListenerList_.clear();
    EXPECT_TRUE(DelayedSingleton<ListenerManagerAdapter>::GetInstance()->StopListener() == ERR_OK);
}

#ifdef STANDBY_SENSORS_SENSOR_ENABLE
/**
 * @tc.name: StandbyPluginUnitTest_032
 * @tc.desc: test MotionSensorMonitor of AcceleromterCallback.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0032, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    SensorEvent event;
    repeatedMotionConstraint->Init();
    GravityData data = {0, 0, 0};
    event.sensorTypeId = SENSOR_TYPE_ID_NONE;
    event.data = reinterpret_cast<uint8_t*>(&data);
    repeatedMotionConstraint->AcceleromterCallback(&event);
    EXPECT_TRUE(repeatedMotionConstraint->GetEnergy() == 0);
    repeatedMotionConstraint->energy_ = 10000;
    repeatedMotionConstraint->AcceleromterCallback(&event);
    repeatedMotionConstraint->AcceleromterCallback(nullptr);
}

/**
 * @tc.name: StandbyPluginUnitTest_033
 * @tc.desc: test MotionSensorMonitor of AcceleromterCallback.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0033, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(PERIODLY_TASK_DECTION_TIMEOUT,
            PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    SensorEvent event;
    repeatedMotionConstraint->Init();
    GravityData data = {0, 0, 0};
    event.sensorTypeId = SENSOR_TYPE_ID_NONE;
    event.data = reinterpret_cast<uint8_t*>(&data);
    repeatedMotionConstraint->RepeatAcceleromterCallback(&event);
    EXPECT_NE(repeatedMotionConstraint->GetEnergy(), 0);
    repeatedMotionConstraint->energy_ = 10000;
    repeatedMotionConstraint->RepeatAcceleromterCallback(&event);
    repeatedMotionConstraint->RepeatAcceleromterCallback(nullptr);
}
#endif

/**
 * @tc.name: StandbyPluginUnitTest_034
 * @tc.desc: test StateManagerAdapter of OnScreenOffHalfHourInner.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0034, TestSize.Level1)
{
    bool scrOffHalfHourCtrl = true;
    bool repeated = true;
    standbyStateManager_->OnScreenOffHalfHourInner(scrOffHalfHourCtrl, repeated);
    repeated = false;
    standbyStateManager_->OnScreenOffHalfHourInner(scrOffHalfHourCtrl, repeated);
    scrOffHalfHourCtrl = false;
    standbyStateManager_->OnScreenOffHalfHourInner(scrOffHalfHourCtrl, repeated);
    repeated = true;
    standbyStateManager_->OnScreenOffHalfHourInner(scrOffHalfHourCtrl, repeated);
    EXPECT_TRUE(standbyStateManager_->scrOffHalfHourCtrl_ == false);

    standbyStateManager_->curStatePtr_ = standbyStateManager_->sleepStatePtr_;
    standbyStateManager_->preStatePtr_ = standbyStateManager_->maintStatePtr_;
    standbyStateManager_->OnScreenOffHalfHourInner(true, false);

    standbyStateManager_->curStatePtr_ = standbyStateManager_->maintStatePtr_;
    standbyStateManager_->preStatePtr_ = standbyStateManager_->sleepStatePtr_;
    standbyStateManager_->OnScreenOffHalfHourInner(true, false);

    standbyStateManager_->curStatePtr_ = standbyStateManager_->workingStatePtr_;
    standbyStateManager_->preStatePtr_ = standbyStateManager_->sleepStatePtr_;
    standbyStateManager_->OnScreenOffHalfHourInner(true, false);
}

/**
 * @tc.name: StandbyPluginUnitTest_035
 * @tc.desc: test StateManagerAdapter of ShellDump.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0035, TestSize.Level1)
{
    std::vector<std::string> argsInStr {};
    std::string result;
    int32_t dumpFirstParam = 0;
    argsInStr.insert(argsInStr.begin() + dumpFirstParam, "0");
    standbyStateManager_->ShellDump(argsInStr, result);
    argsInStr.insert(argsInStr.begin() + dumpFirstParam, "-D");
    standbyStateManager_->ShellDump(argsInStr, result);
    argsInStr.insert(argsInStr.begin() + dumpFirstParam, "-E");
    standbyStateManager_->ShellDump(argsInStr, result);
    argsInStr.insert(argsInStr.begin() + dumpFirstParam, "-S");
    standbyStateManager_->ShellDump(argsInStr, result);
    EXPECT_TRUE(argsInStr.size() != 0);
}

/**
 * @tc.name: StandbyPluginUnitTest_036
 * @tc.desc: test StateManagerAdapter of DumpEnterSpecifiedState.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0036, TestSize.Level1)
{
    std::vector<std::string> argsInStr {};
    std::string result;
    int32_t dumpFirstParam = 0;
    int32_t dumpThirdParam = 2;
    argsInStr.insert(argsInStr.begin() + dumpFirstParam, "0");
    argsInStr.insert(argsInStr.begin() + dumpFirstParam, "0");
    argsInStr.insert(argsInStr.begin() + dumpThirdParam, "false");
    standbyStateManager_->DumpEnterSpecifiedState(argsInStr, result);
    argsInStr.insert(argsInStr.begin() + dumpThirdParam, "0");
    standbyStateManager_->DumpEnterSpecifiedState(argsInStr, result);
    EXPECT_TRUE(argsInStr.size() != 0);
}

/**
 * @tc.name: StandbyPluginUnitTest_037
 * @tc.desc: test StateManagerAdapter of DumpActivateMotion.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0037, TestSize.Level1)
{
    std::vector<std::string> argsInStr {};
    std::string result;
    int32_t dumpFirstParam = 0;
    int32_t dumpSecondParam = 1;
    argsInStr.insert(argsInStr.begin() + dumpFirstParam, "0");
    argsInStr.insert(argsInStr.begin() + dumpFirstParam, "0");
    argsInStr.insert(argsInStr.begin() + dumpSecondParam, "--motion");
    standbyStateManager_->DumpActivateMotion(argsInStr, result);
    argsInStr.insert(argsInStr.begin() + dumpSecondParam, "--blocked");
    standbyStateManager_->DumpActivateMotion(argsInStr, result);
    argsInStr.insert(argsInStr.begin() + dumpSecondParam, "--halfhour");
    standbyStateManager_->DumpActivateMotion(argsInStr, result);
    argsInStr.insert(argsInStr.begin() + dumpSecondParam, "0");
    standbyStateManager_->DumpActivateMotion(argsInStr, result);
    EXPECT_TRUE(argsInStr.size() != 0);
}

#ifdef STANDBY_MULTIMODALINPUT_INPUT_ENABLE
/**
 * @tc.name: StandbyPluginUnitTest_038
 * @tc.desc: test InputManagerListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_0038, TestSize.Level1)
{
    auto inputManagerListener = std::make_shared<InputManagerListener>();
    inputManagerListener->subscriberId_ = -1;
    inputManagerListener->StopListener();
    inputManagerListener->OnCallbackEvent(MMI::SwitchEvent::SWITCH_OFF);
    inputManagerListener->OnCallbackEvent(MMI::SwitchEvent::SWITCH_ON);
    EXPECT_TRUE(inputManagerListener->subscriberId_ <= 0);
}
#endif

#ifdef ENABLE_BACKGROUND_TASK_MGR
/**
 * @tc.name: StandbyPluginUnitTest_039
 * @tc.desc: test BackgroundTaskListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_039, TestSize.Level1)
{
    auto backgroundTaskListener = std::make_shared<BackgroundTaskListener>();
    backgroundTaskListener->StartListener();
    backgroundTaskListener->StopListener();
    backgroundTaskListener->bgTaskListenerImpl_ = nullptr;
    backgroundTaskListener->StartListener();
    EXPECT_NE(backgroundTaskListener->StopListener(), ERR_OK);
}
#endif

/**
 * @tc.name: StandbyPluginUnitTest_040
 * @tc.desc: test ListenerManagerAdapter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_040, TestSize.Level1)
{
    StandbyConfigManager::GetInstance()->strategyList_.emplace_back("RUNNING_LOCK");
    listenerManager_->StopListener();
    listenerManager_->UnInit();
    listenerManager_->StartListener();
    listenerManager_->Init();
    StandbyConfigManager::GetInstance()->strategyList_.emplace_back("NETWORK");
    listenerManager_->StopListener();
    listenerManager_->UnInit();
    listenerManager_->StartListener();
    listenerManager_->Init();
    EXPECT_FALSE(listenerManager_->listenerPluginMap_.empty());
}

/**
 * @tc.name: StandbyPluginUnitTest_041
 * @tc.desc: test StartTransitNextState.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_041, TestSize.Level1)
{
    standbyStateManager_->sleepStatePtr_->stateManager_.reset();
    standbyStateManager_->sleepStatePtr_->TransitToPhaseInner(0, 0);
    standbyStateManager_->sleepStatePtr_->StartTransitNextState(standbyStateManager_->sleepStatePtr_);
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->sleepStatePtr_->stateManager_ = standbyStateManager_;
    standbyStateManager_->isEvalution_ = true;
    standbyStateManager_->sleepStatePtr_->nextState_ = 0;
    standbyStateManager_->sleepStatePtr_->StartTransitNextState(standbyStateManager_->sleepStatePtr_);
    SleepForFC();
    EXPECT_FALSE(standbyStateManager_->sleepStatePtr_ == nullptr);
}

/**
 * @tc.name: StandbyPluginUnitTest_042
 * @tc.desc: test StopTimedTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_042, TestSize.Level1)
{
    standbyStateManager_->workingStatePtr_->timedTaskMap_.emplace("", -1);
    standbyStateManager_->workingStatePtr_->StopTimedTask("");
    standbyStateManager_->workingStatePtr_->StopTimedTask("test");
    standbyStateManager_->workingStatePtr_->DestroyAllTimedTask();
    SleepForFC();
    EXPECT_TRUE(standbyStateManager_->workingStatePtr_->timedTaskMap_.empty());

    standbyStateManager_->workingStatePtr_->stateManager_.reset();
    standbyStateManager_->workingStatePtr_->EndState();
    standbyStateManager_->workingStatePtr_->EndEvalCurrentState(false);
    standbyStateManager_->workingStatePtr_->stateManager_ = standbyStateManager_;
}

/**
 * @tc.name: StandbyPluginUnitTest_043
 * @tc.desc: test StopTimedTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_043, TestSize.Level1)
{
    standbyStateManager_->workingStatePtr_->timedTaskMap_.emplace("", -1);
    standbyStateManager_->workingStatePtr_->StopTimedTask("");
    standbyStateManager_->workingStatePtr_->StopTimedTask("test");
    standbyStateManager_->workingStatePtr_->DestroyAllTimedTask();
    SleepForFC();
    EXPECT_TRUE(standbyStateManager_->workingStatePtr_->timedTaskMap_.empty());
}

/**
 * @tc.name: StandbyPluginUnitTest_044
 * @tc.desc: test SleepState.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyPluginUnitTest, StandbyPluginUnitTest_044, TestSize.Level1)
{
    std::string result {""};
    standbyStateManager_->sleepStatePtr_->ShellDump({"-D", "--repeat"}, result);
    standbyStateManager_->sleepStatePtr_->ShellDump({"-S", "--r"}, result);
    standbyStateManager_->sleepStatePtr_->ShellDump({"-S", "--repeat"}, result);
    standbyStateManager_->sleepStatePtr_->EndEvalCurrentState(false);
    SleepForFC();
    EXPECT_TRUE(standbyStateManager_->workingStatePtr_->timedTaskMap_.empty());
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
