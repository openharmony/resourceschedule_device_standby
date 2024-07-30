/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "standby_service_impl.h"
#ifdef ENABLE_BACKGROUND_TASK_MGR
#include "resource_type.h"
#endif
#include "system_ability_definition.h"

#include "mock_bundle_manager_helper.h"

using namespace testing::ext;
using namespace testing;
using testing::Return;
using testing::DoAll;

namespace OHOS {
namespace DevStandbyMgr {
static constexpr int32_t NETWORK_INDEX = 101;
static constexpr int32_t TIMER_INDEX = 103;
static constexpr int32_t EXEMPT_ALL_RESOURCES = 100;

class MockStandbyServiceUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static std::shared_ptr<MockBundleManagerHelper> bundleManagerHelperMock_;
};

std::shared_ptr<MockBundleManagerHelper> MockStandbyServiceUnitTest::bundleManagerHelperMock_ = nullptr;

void MockStandbyServiceUnitTest::TearDownTestCase()
{
    bundleManagerHelperMock_.reset();
}

void MockStandbyServiceUnitTest::SetUp()
{
    bundleManagerHelperMock_ = std::make_shared<MockBundleManagerHelper>();
    SetBundleManagerHelper(bundleManagerHelperMock_);
}

void MockStandbyServiceUnitTest::TearDown()
{
    CleanBundleManagerHelper();
}

/**
 * @tc.name: GetExemptedResourceType_001
 * @tc.desc: should return 0 when failed to get application info
 * @tc.type: FUNC
 */
HWTEST_F(MockStandbyServiceUnitTest, GetExemptedResourceType_001, TestSize.Level1)
{
    EXPECT_CALL(*bundleManagerHelperMock_, GetApplicationInfo(_, _, _, _)).WillOnce(Return(false));
    auto exemptedType = StandbyServiceImpl::GetInstance()->GetExemptedResourceType(MAX_ALLOW_TYPE_NUMBER);
    EXPECT_EQ(exemptedType, 0u);
}

/**
 * @tc.name: GetExemptedResourceType_002
 * @tc.desc: should return 0 when resource type is inconsistence with configuration
 * @tc.type: FUNC
 */
HWTEST_F(MockStandbyServiceUnitTest, GetExemptedResourceType_002, TestSize.Level1)
{
    AppExecFwk::ApplicationInfo info {};
    info.resourcesApply = { TIMER_INDEX };
    EXPECT_CALL(*bundleManagerHelperMock_, GetApplicationInfo(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(info), Return(true)));
    auto exemptedType = StandbyServiceImpl::GetInstance()->GetExemptedResourceType(AllowType::NETWORK);
    EXPECT_EQ(exemptedType, 0u);
}

/**
 * @tc.name: GetExemptedResourceType_003
 * @tc.desc: should return origin number when resource list contain 0
 * @tc.type: FUNC
 */
HWTEST_F(MockStandbyServiceUnitTest, GetExemptedResourceType_003, TestSize.Level1)
{
    AppExecFwk::ApplicationInfo info {};
    info.resourcesApply = { EXEMPT_ALL_RESOURCES };
    EXPECT_CALL(*bundleManagerHelperMock_, GetApplicationInfo(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(info), Return(true)));
    auto exemptedType = StandbyServiceImpl::GetInstance()->GetExemptedResourceType(MAX_ALLOW_TYPE_NUMBER);
    EXPECT_EQ(exemptedType, MAX_ALLOW_TYPE_NUMBER);
}

/**
 * @tc.name: GetExemptedResourceType_004
 * @tc.desc: should return CPU when resource list contain CPU_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(MockStandbyServiceUnitTest, GetExemptedResourceType_004, TestSize.Level1)
{
    AppExecFwk::ApplicationInfo info {};
    info.resourcesApply = { NETWORK_INDEX };
    EXPECT_CALL(*bundleManagerHelperMock_, GetApplicationInfo(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(info), Return(true)));
    auto exemptedType = StandbyServiceImpl::GetInstance()->GetExemptedResourceType(MAX_ALLOW_TYPE_NUMBER);
    EXPECT_EQ(exemptedType, AllowType::NETWORK);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
