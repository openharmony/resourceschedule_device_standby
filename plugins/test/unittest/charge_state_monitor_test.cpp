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

#include "gtest/gtest.h"
#include "charge_state_monitor.h"
#include "constraint_manager_adapter.h"
#include "state_manager_adapter.h"
#include "standby_service_impl.h"

using namespace testing::ext;

namespace OHOS {
namespace DevStandbyMgr {
class ChargeStateMonitorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override {}
};

void ChargeStateMonitorTest::TearDownTestCase() {}

void ChargeStateMonitorTest::SetUpTestCase() {}

void ChargeStateMonitorTest::SetUp()
{
    StandbyServiceImpl::GetInstance()->constraintManager_ = std::make_shared<ConstraintManagerAdapter>();
    StandbyServiceImpl::GetInstance()->standbyStateManager_ = std::make_shared<StateManagerAdapter>();
}

/**
 * @tc.name: Init
 * @tc.desc: test Init.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ChargeStateMonitorTest, Init, TestSize.Level1)
{
    std::shared_ptr<IConstraintMonitor> chargeMonitor = std::make_shared<ChargeStateMonitor>();
    bool ret = chargeMonitor->Init();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: StartMonitoring
 * @tc.desc: test StartMonitoring.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ChargeStateMonitorTest, StartMonitoring, TestSize.Level1)
{
    std::shared_ptr<IConstraintMonitor> chargeMonitor = std::make_shared<ChargeStateMonitor>();
    chargeMonitor->StartMonitoring();
}

/**
 * @tc.name: StopMonitoring
 * @tc.desc: test StopMonitoring.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ChargeStateMonitorTest, StopMonitoring, TestSize.Level1)
{
    std::shared_ptr<IConstraintMonitor> chargeMonitor = std::make_shared<ChargeStateMonitor>();
    chargeMonitor->StopMonitoring();
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
