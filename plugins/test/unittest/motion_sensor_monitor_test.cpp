/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "motion_sensor_monitor.h"
#include "common_constant.h"
#include "state_manager_adapter.h"
#include "constraint_manager_adapter.h"
#include "standby_config_manager.h"
#include "standby_service_impl.h"
#include "dark_state.h"

using namespace testing::ext;

namespace OHOS {
namespace DevStandbyMgr {
class MotionSensorMonitorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override {}
};

void MotionSensorMonitorTest::TearDownTestCase() {}

void MotionSensorMonitorTest::SetUpTestCase() {}

void MotionSensorMonitorTest::SetUp()
{
    StandbyServiceImpl::GetInstance()->standbyStateManager_ =
        std::make_shared<IStateManagerAdapter>(new StateManagerAdapter());
    StandbyServiceImpl::GetInstance()->constraintManager_ =
        std::make_shared<IConstraintManagerAdapter>(new ConstraintManagerAdapter());
    StandbyServiceImpl::GetInstance()->Init();
}

/**
 * @tc.name: Init
 * @tc.desc: test Init.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, Init, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    bool ret = repeatedMotionConstraint->Init();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AcceleromterCallback
 * @tc.desc: test AcceleromterCallback.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, AcceleromterCallback, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    StandbyConfigManager::GetInstance()->standbyParaMap_[MOTION_THREADSHOLD] = 0;
    SensorEvent event;
    GravityData data = { 0, 0, 0 };
    event.sensorTypeId = SENSOR_TYPE_ID_NONE;
    event.data = reinterpret_cast<uint8_t *>(&data);
    repeatedMotionConstraint->energy_ = 10000;
    repeatedMotionConstraint->AcceleromterCallback(&event);
    repeatedMotionConstraint->AcceleromterCallback(nullptr);
}

/**
 * @tc.name: RepeatAcceleromterCallback
 * @tc.desc: test RepeatAcceleromterCallback.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, RepeatAcceleromterCallback, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    StandbyConfigManager::GetInstance()->standbyParaMap_[MOTION_THREADSHOLD] = 0;
    SensorEvent event;
    GravityData data = { 0, 0, 0 };
    event.sensorTypeId = SENSOR_TYPE_ID_NONE;
    event.data = reinterpret_cast<uint8_t *>(&data);
    repeatedMotionConstraint->RepeatAcceleromterCallback(&event);
    repeatedMotionConstraint->energy_ = 10000;
    repeatedMotionConstraint->RepeatAcceleromterCallback(&event);
    repeatedMotionConstraint->RepeatAcceleromterCallback(nullptr);
}

/**
 * @tc.name: MotionSensorCallback
 * @tc.desc: test MotionSensorCallback.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, MotionSensorCallback, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->MotionSensorCallback(nullptr);
}

/**
 * @tc.name: SetEnergy
 * @tc.desc: test SetEnergy.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, SetEnergy, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    double energy = 10000;
    repeatedMotionConstraint->SetEnergy(energy);
    EXPECT_EQ(repeatedMotionConstraint->GetEnergy(), energy);
}

/**
 * @tc.name: AssignAcclerometerSensorCallBack
 * @tc.desc: test AssignAcclerometerSensorCallBack001.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, AssignAcclerometerSensorCallBack001, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->sensorUserMap_.clear();
    repeatedMotionConstraint->AssignAcclerometerSensorCallBack();
}

/**
 * @tc.name: AssignAcclerometerSensorCallBack
 * @tc.desc: test AssignAcclerometerSensorCallBack002.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, AssignAcclerometerSensorCallBack002, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    repeatedMotionParams.isRepeatedDetection_ = false;
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->sensorUserMap_.clear();
    repeatedMotionConstraint->sensorUserMap_.emplace(SENSOR_TYPE_ID_ACCELEROMETER, SensorUser{});
    repeatedMotionConstraint->AssignAcclerometerSensorCallBack();
}

/**
 * @tc.name: AssignAcclerometerSensorCallBack
 * @tc.desc: test AssignAcclerometerSensorCallBack003.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, AssignAcclerometerSensorCallBack003, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    repeatedMotionParams.isRepeatedDetection_ = true;
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->sensorUserMap_.clear();
    repeatedMotionConstraint->sensorUserMap_.emplace(SENSOR_TYPE_ID_ACCELEROMETER, SensorUser{});
    repeatedMotionConstraint->AssignAcclerometerSensorCallBack();
}

/**
 * @tc.name: AssignMotionSensorCallBack
 * @tc.desc: test AssignMotionSensorCallBack001.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, AssignMotionSensorCallBack001, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->sensorUserMap_.clear();
    repeatedMotionConstraint->sensorUserMap_.emplace(SENSOR_TYPE_ID_SIGNIFICANT_MOTION, SensorUser{});
    repeatedMotionConstraint->AssignMotionSensorCallBack();
}

/**
 * @tc.name: AssignMotionSensorCallBack
 * @tc.desc: test AssignMotionSensorCallBack002.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, AssignMotionSensorCallBack002, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->sensorUserMap_.clear();
    repeatedMotionConstraint->AssignMotionSensorCallBack();
}

/**
 * @tc.name: StartMonitoring
 * @tc.desc: test StartMonitoring.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, StartMonitoring, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->StartMonitoring();
}

/**
 * @tc.name: StopMonitoring
 * @tc.desc: test StopMonitoring.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, StopMonitoring, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->StopMonitoring();
}

/**
 * @tc.name: StartSensor
 * @tc.desc: test StartSensor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, StartSensor, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->Init();
    repeatedMotionConstraint->StartSensor();
}

/**
 * @tc.name: StopSensor
 * @tc.desc: test StopSensor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MotionSensorMonitorTest, StopSensor, TestSize.Level1)
{
    ConstraintEvalParam repeatedMotionParams{};
    auto repeatedMotionConstraint = std::make_shared<MotionSensorMonitor>(
        PERIODLY_TASK_DECTION_TIMEOUT, PERIODLY_TASK_REST_TIMEOUT, PERIODLY_TASK_TOTAL_TIMEOUT, repeatedMotionParams);
    repeatedMotionConstraint->Init();
    repeatedMotionConstraint->isMonitoring_ = true;
    repeatedMotionConstraint->StopSensor();
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
