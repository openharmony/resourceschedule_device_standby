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
class ConstraintManagerAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override {}
};

void ConstraintManagerAdapterTest::TearDownTestCase() {}

void ConstraintManagerAdapterTest::SetUpTestCase() {}

void ConstraintManagerAdapterTest::SetUp()
{
    StandbyServiceImpl::GetInstance()->standbyStateManager_ = std::make_shared<StateManagerAdapter>();
}

/**
 * @tc.name: Init
 * @tc.desc: test Init.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, Init, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    bool ret = constraintManagerAdapter->Init();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UnInit
 * @tc.desc: test UnInit.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, UnInit, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    bool ret = constraintManagerAdapter->UnInit();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: StartEvalution
 * @tc.desc: test StartEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, StartEvalution001, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    ConstraintEvalParam params;
    constraintManagerAdapter->isEvaluation_ = true;
    constraintManagerAdapter->StartEvalution(params);
}

/**
 * @tc.name: StartEvalution
 * @tc.desc: test StartEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, StartEvalution002, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    ConstraintEvalParam params;
    constraintManagerAdapter->isEvaluation_ = false;
    constraintManagerAdapter->constraintMap_.clear();
    std::shared_ptr<StateManagerAdapter> stateManagerAdapter = std::make_shared<StateManagerAdapter>();
    stateManagerAdapter->isEvalution_ = false;
    constraintManagerAdapter->stateManager_ = stateManagerAdapter;
    constraintManagerAdapter->StartEvalution(params);
}

/**
 * @tc.name: StartEvalution
 * @tc.desc: test StartEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, StartEvalution003, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    ConstraintEvalParam params;
    constraintManagerAdapter->isEvaluation_ = false;
    std::shared_ptr<ChargeStateMonitor> monitor = nullptr;
    constraintManagerAdapter->RegisterConstraintCallback(params, monitor);
    EXPECT_EQ(constraintManagerAdapter->constraintMap_.size(), 1);
    ErrCode ret = constraintManagerAdapter->StartEvalution(params);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: StartEvalution
 * @tc.desc: test StartEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, StartEvalution004, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    ConstraintEvalParam params;
    constraintManagerAdapter->isEvaluation_ = false;
    std::shared_ptr<ChargeStateMonitor> monitor = std::make_shared<ChargeStateMonitor>();
    constraintManagerAdapter->RegisterConstraintCallback(params, monitor);
    constraintManagerAdapter->StartEvalution(params);
}

/**
 * @tc.name: StopEvalution
 * @tc.desc: test StopEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, StopEvalution001, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    constraintManagerAdapter->isEvaluation_ = false;
    constraintManagerAdapter->StopEvalution();
}

/**
 * @tc.name: StopEvalution
 * @tc.desc: test StopEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, StopEvalution002, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    constraintManagerAdapter->isEvaluation_ = true;
    constraintManagerAdapter->curMonitor_ = nullptr;
    constraintManagerAdapter->StopEvalution();
}

/**
 * @tc.name: StopEvalution
 * @tc.desc: test StopEvalution.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, StopEvalution003, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    constraintManagerAdapter->isEvaluation_ = true;
    constraintManagerAdapter->curMonitor_ = std::make_shared<ChargeStateMonitor>();
    constraintManagerAdapter->StopEvalution();
}

/**
 * @tc.name: RegisterConstraintCallback
 * @tc.desc: test RegisterConstraintCallback.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, RegisterConstraintCallback, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    ConstraintEvalParam params;
    std::shared_ptr<ChargeStateMonitor> monitor = nullptr;
    constraintManagerAdapter->RegisterConstraintCallback(params, monitor);
}

/**
 * @tc.name: ShellDump
 * @tc.desc: test StopMonitoring.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstraintManagerAdapterTest, ShellDump, TestSize.Level1)
{
    std::shared_ptr<ConstraintManagerAdapter> constraintManagerAdapter = std::make_shared<ConstraintManagerAdapter>();
    std::vector<std::string> argsInStr;
    string result;
    constraintManagerAdapter->ShellDump(argsInStr, result);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
