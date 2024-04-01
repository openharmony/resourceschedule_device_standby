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
#include "istandby_service.h"
#include "resourcce_request.h"
#include "standby_ipc_interface_code.h"
#include "standby_service_client.h"
#include "standby_service_proxy.h"
#include "standby_service_subscriber_stub.h"
#include "standby_service_subscriber_proxy.h"

using namespace testing::ext;

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
 * @tc.name: StandbyServiceClientUnitTest_001
 * @tc.desc: test SubscribeStandbyCallback.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_001, TestSize.Level1)
{
    sptr<IStandbyServiceSubscriber> nullSubscriber = nullptr;
    EXPECT_NE(StandbyServiceClient::GetInstance().SubscribeStandbyCallback(nullSubscriber), ERR_OK);
    EXPECT_NE(StandbyServiceClient::GetInstance().SubscribeStandbyCallback(nullSubscriber), ERR_OK);
    StandbyServiceClient::GetInstance().UnsubscribeStandbyCallback(nullSubscriber);
    sptr<IStandbyServiceSubscriber> subscriber = new (std::nothrow) StandbyServiceSubscriberStub();
    EXPECT_NE(StandbyServiceClient::GetInstance().SubscribeStandbyCallback(subscriber), ERR_OK);
    StandbyServiceClient::GetInstance().SubscribeStandbyCallback(subscriber);
    EXPECT_NE(StandbyServiceClient::GetInstance().UnsubscribeStandbyCallback(subscriber), ERR_OK);
    EXPECT_NE(StandbyServiceClient::GetInstance().UnsubscribeStandbyCallback(subscriber), ERR_OK);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_002
 * @tc.desc: test ApplyAllowResource.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_002, TestSize.Level1)
{
    sptr<ResourceRequest> nullRequest = nullptr;
    EXPECT_NE(StandbyServiceClient::GetInstance().ApplyAllowResource(nullRequest), ERR_OK);
    EXPECT_NE(StandbyServiceClient::GetInstance().UnapplyAllowResource(nullRequest), ERR_OK);

    sptr<ResourceRequest> resouarceRequest = new (std::nothrow) ResourceRequest();
    EXPECT_NE(StandbyServiceClient::GetInstance().ApplyAllowResource(resouarceRequest), ERR_OK);
    EXPECT_NE(StandbyServiceClient::GetInstance().UnapplyAllowResource(resouarceRequest), ERR_OK);

    sptr<ResourceRequest> validResRequest = new (std::nothrow) ResourceRequest(AllowType::NETWORK,
        0, "test_process", 100, "test", 1);
    EXPECT_EQ(StandbyServiceClient::GetInstance().ApplyAllowResource(validResRequest), ERR_OK);
    EXPECT_EQ(StandbyServiceClient::GetInstance().UnapplyAllowResource(validResRequest), ERR_OK);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_003
 * @tc.desc: test GetAllowList.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_003, TestSize.Level1)
{
    std::vector<AllowInfo> allowInfoList;
    sptr<ResourceRequest> nullRequest = nullptr;
    EXPECT_EQ(StandbyServiceClient::GetInstance().GetAllowList(AllowType::NETWORK, allowInfoList, 0), ERR_OK);
    EXPECT_NE(StandbyServiceClient::GetInstance().GetAllowList(0, allowInfoList, 0), ERR_OK);
    EXPECT_EQ(StandbyServiceClient::GetInstance().GetAllowList((1 << 6), allowInfoList, 0), ERR_OK);
    allowInfoList.emplace_back(AllowInfo {});
    StandbyServiceClient::GetInstance().GetAllowList((1 << 6), allowInfoList, 0);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_004
 * @tc.desc: test IsDeviceInStandby.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_004, TestSize.Level1)
{
    bool isStandby {false};
    EXPECT_EQ(StandbyServiceClient::GetInstance().IsDeviceInStandby(isStandby), ERR_OK);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_005
 * @tc.desc: test Unmarshalling.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_005, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_EQ(AllowInfo::Unmarshalling(data), nullptr);
    EXPECT_EQ(ResourceRequest::Unmarshalling(data), nullptr);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_006
 * @tc.desc: test ResetStandbyServiceClient.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_006, TestSize.Level1)
{
    StandbyServiceClient::GetInstance().ResetStandbyServiceClient();
    StandbyServiceClient::GetInstance().ResetStandbyServiceClient();
    StandbyServiceClient::GetInstance().GetStandbyServiceProxy();
    StandbyServiceClient::GetInstance().ResetStandbyServiceClient();
    EXPECT_EQ(StandbyServiceClient::GetInstance().standbyServiceProxy_, nullptr);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_007
 * @tc.desc: test StandbyServiceSubscriberStub.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_007, TestSize.Level1)
{
    sptr<StandbyServiceSubscriberStub> subscriber = new (std::nothrow) StandbyServiceSubscriberStub();
    MessageParcel data {};
    MessageParcel reply {};
    MessageOption option {};
    subscriber->OnRemoteRequestInner(
        (static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_ALLOW_LIST_CHANGED)), data, reply, option);
    subscriber->OnRemoteRequestInner(
        (static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_ALLOW_LIST_CHANGED)) + 1, data, reply, option);
    EXPECT_NE(subscriber->HandleOnDeviceIdleMode(data), ERR_OK);
    subscriber->HandleOnAllowListChanged(data);
    data.WriteBool(false);
    subscriber->HandleOnDeviceIdleMode(data);
    data.WriteBool(false);
    subscriber->HandleOnDeviceIdleMode(data);
    subscriber->HandleOnAllowListChanged(data);
    MessageParcel allowListData {};
    subscriber->HandleOnAllowListChanged(allowListData);
    allowListData.WriteInt32(0);
    subscriber->HandleOnAllowListChanged(allowListData);
    allowListData.WriteInt32(0);
    subscriber->HandleOnAllowListChanged(allowListData);
    allowListData.WriteString("");
    subscriber->HandleOnAllowListChanged(allowListData);
    allowListData.WriteUint32(0);
    subscriber->HandleOnAllowListChanged(allowListData);
    allowListData.WriteBool(false);
    subscriber->HandleOnAllowListChanged(allowListData);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_008
 * @tc.desc: test StandbyServiceProxy.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_008, TestSize.Level1)
{
    sptr<IRemoteObject> impl {};
    sptr<StandbyServiceProxy> proxy = new (std::nothrow) StandbyServiceProxy(impl);
    sptr<IStandbyServiceSubscriber> nullSubscriber = nullptr;
    EXPECT_NE(proxy->SubscribeStandbyCallback(nullSubscriber), ERR_OK);
    EXPECT_NE(proxy->SubscribeStandbyCallback(nullSubscriber), ERR_OK);
    proxy->UnsubscribeStandbyCallback(nullSubscriber);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_009
 * @tc.desc: test ReportWorkSchedulerStatus.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_009, TestSize.Level1)
{
    EXPECT_EQ(StandbyServiceClient::GetInstance().ReportWorkSchedulerStatus(true, -1, ""), ERR_OK);
    EXPECT_EQ(StandbyServiceClient::GetInstance().ReportWorkSchedulerStatus(false, -1, ""), ERR_OK);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_010
 * @tc.desc: test StandbyServiceSubscriberProxy.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_010, TestSize.Level1)
{
    sptr<IRemoteObject> impl {};
    sptr<StandbyServiceSubscriberProxy> proxy = new (std::nothrow) StandbyServiceSubscriberProxy(impl);
    sptr<StandbyServiceSubscriberProxy> nullSubscriber = nullptr;
    proxy->OnDeviceIdleMode(false, false);
    proxy->OnAllowListChanged(-1, "", 0, false);
    EXPECT_NE(proxy, nullptr);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_011
 * @tc.desc: test IsStrategyEnabled.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_011, TestSize.Level1)
{
    std::string strategyName;
    bool isEnabled =  false;
    EXPECT_EQ(StandbyServiceClient::GetInstance().IsStrategyEnabled(strategyName, isEnabled), ERR_OK);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_012
 * @tc.desc: test IsStrategyEnabled.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_012, TestSize.Level1)
{
    DeviceStateType type = DeviceStateType::WIFI_P2P_CHANGE;
    bool enabled = false;
    EXPECT_EQ(StandbyServiceClient::GetInstance().ReportDeviceStateChanged(type, enabled), ERR_OK);
}

/**
 * @tc.name: StandbyServiceClientUnitTest_013
 * @tc.desc: test IsStrategyEnabled.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyServiceClientUnitTest, StandbyServiceClientUnitTest_013, TestSize.Level1)
{
    uint32_t restrictType = 1;
    std::vector<AllowInfo> restrictInfoList;
    uint32_t reasonCode = ReasonCodeEnum::REASON_APP_API;
    EXPECT_EQ(StandbyServiceClient::GetInstance().GetRestrictList(restrictType, restrictInfoList, reasonCode), ERR_OK);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
