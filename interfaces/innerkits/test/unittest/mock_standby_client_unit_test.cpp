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
#include "singleton.h"

#include "allow_type.h"
#include "standby_service_client.h"
#include "standby_service_subscriber_stub.h"
using namespace testing::ext;

extern void MockGetSystemAbilityManager(bool mockRet);
extern void MockSendRequest(bool mockRet);
namespace OHOS {
namespace DevStandbyMgr {

class StandbyServiceClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: MockStandbyServiceClientUnitTest_001
 * @tc.desc: test MockFunction.
 * @tc.type: FUNC
 * @tc.require: AR000HQ76V
 */
HWTEST_F(StandbyServiceClientUnitTest, MockStandbyServiceClientUnitTest_001, TestSize.Level1)
{
    MockGetSystemAbilityManager(false);
    sptr<IStandbyServiceSubscriber> nullSubscriber = nullptr;
    EXPECT_NE(StandbyServiceClient::GetInstance().SubscribeStandbyCallback(nullSubscriber), ERR_OK);
    EXPECT_NE(StandbyServiceClient::GetInstance().UnsubscribeStandbyCallback(nullSubscriber), ERR_OK);
    sptr<ResourceRequest> nullRequest = nullptr;
    EXPECT_NE(StandbyServiceClient::GetInstance().ApplyAllowResource(nullRequest), ERR_OK);
    EXPECT_NE(StandbyServiceClient::GetInstance().UnapplyAllowResource(nullRequest), ERR_OK);
    std::vector<AllowInfo> allowInfoList;
    nullRequest = nullptr;
    EXPECT_NE(StandbyServiceClient::GetInstance().GetAllowList(AllowType::NET, allowInfoList, 0), ERR_OK);
    EXPECT_NE(StandbyServiceClient::GetInstance().GetAllowList(0, allowInfoList, 0), ERR_OK);
    bool isStandby {false};
    EXPECT_NE(StandbyServiceClient::GetInstance().IsDeviceInStandby(isStandby), ERR_OK);
    MockGetSystemAbilityManager(true);
    StandbyServiceClient::GetInstance().IsDeviceInStandby(isStandby);
}

/**
 * @tc.name: MockStandbyServiceClientUnitTest_002
 * @tc.desc: test StandbyServiceProxy.
 * @tc.type: FUNC
 * @tc.require: AR000HQ76V
 */
HWTEST_F(StandbyServiceClientUnitTest, MockStandbyServiceClientUnitTest_002, TestSize.Level1)
{
    sptr<IRemoteObject> impl {};
    sptr<StandbyServiceProxy> proxy = new (std::nothrow) StandbyServiceProxy(impl);
    MockSendRequest(true);
    sptr<IStandbyServiceSubscriber> subscriber = new (std::nothrow) StandbyServiceSubscriberStub();
    EXPECT_NE(proxy->SubscribeStandbyCallback(subscriber), ERR_OK);
    EXPECT_NE(proxy->UnsubscribeStandbyCallback(subscriber), ERR_OK);
    sptr<ResourceRequest> resouarceRequest = new (std::nothrow) ResourceRequest();
    EXPECT_NE(proxy->ApplyAllowResource(resouarceRequest), ERR_OK);
    EXPECT_NE(proxy->UnapplyAllowResource(resouarceRequest), ERR_OK);
    std::vector<AllowInfo> allowInfoList;
    EXPECT_NE(proxy->GetAllowList(0, allowInfoList, 0), ERR_OK);
    bool isStandby {false};
    proxy->IsDeviceInStandby(isStandby);

    MockSendRequest(false);
    EXPECT_NE(proxy->SubscribeStandbyCallback(subscriber), ERR_OK);
    EXPECT_NE(proxy->UnsubscribeStandbyCallback(subscriber), ERR_OK);
    EXPECT_NE(proxy->ApplyAllowResource(resouarceRequest), ERR_OK);
    EXPECT_NE(proxy->UnapplyAllowResource(resouarceRequest), ERR_OK);
    EXPECT_NE(proxy->GetAllowList(0, allowInfoList, 0), ERR_OK);
    proxy->IsDeviceInStandby(isStandby);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS