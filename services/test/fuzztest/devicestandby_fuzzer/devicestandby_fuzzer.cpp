/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "devicestandby_fuzzer.h"
#include "securec.h"
#ifdef DEVICE_STANDBY_ACCESS_TOKEN_ENABLE
#include "access_token.h"
#include "accesstoken_kit.h"
#include "access_token_error.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#endif
#include "resourcce_request.h"
#include "istandby_service_subscriber.h"
#include "standby_service.h"
#include "istandby_ipc_inteface_code.h"
#include "standby_service_log.h"
#include "standby_service_subscriber_stub.h"

namespace OHOS {
namespace DevStandbyMgr {
    constexpr size_t U32_AT_SIZE = 17;
    constexpr uint32_t MAX_CODE = 13;
    constexpr uint32_t CONSTANT_ONE = 1;
    constexpr int32_t CONSTANT_TWO = 2;
    const std::string STR_TEST = "test";
    const std::string RSS_NAME = "resource_schedule_service";
    const std::u16string DEVICE_STANDBY_TOKEN = u"ohos.resourceschedule.IStandbyService";
    bool g_initFlag = false;
    sptr<IStandbyServiceSubscriber> subscriber = new StandbyServiceSubscriberStub();
    std::unique_ptr<ResourceRequest> resourceRequest = std::make_unique<ResourceRequest>(CONSTANT_ONE, CONSTANT_TWO,
        STR_TEST, CONSTANT_TWO, STR_TEST, CONSTANT_ONE);
    const uint8_t *g_baseFuzzData = nullptr;
    size_t g_baseFuzzSize = 0;
    size_t g_baseFuzzPos;
    bool g_paramBool;
    int32_t g_paramInt32;
    uint32_t g_paramUint32;
    uint64_t g_paramUint64;
    std::string g_paramString;

    template <class T> T GetData()
    {
        T object{};
        size_t objectSize = sizeof(object);
        if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
            return object;
        }
        errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
        if (ret != EOK) {
            return {};
        }
        g_baseFuzzPos += objectSize;
        return object;
    }

    void InitParam()
    {
        g_paramBool = GetData<bool>();
        g_paramInt32 = GetData<int32_t>();
        g_paramUint32 = GetData<uint32_t>();
        g_paramUint64 = GetData<uint64_t>();
        std::string strParam((const char *) g_baseFuzzData + g_baseFuzzPos, g_baseFuzzSize - g_baseFuzzPos);
        g_paramString = strParam;
    }

    void CoverageHandleIsStrategyEnabled()
    {
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option = {MessageOption::TF_SYNC};
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::IS_STRATEGY_ENABLED),
            datas, reply, option);
        datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::IS_STRATEGY_ENABLED),
            datas, reply, option);
        datas.WriteString(g_paramString);
        datas.RewindRead(0);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::IS_STRATEGY_ENABLED),
            datas, reply, option);
    }

    void CoverageHandleReportWorkSchedulerStatus()
    {
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option = {MessageOption::TF_SYNC};
        datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::REPORT_WORK_SCHEDULER_STATUS),
            datas, reply, option);
        datas.WriteBool(g_paramBool);
        datas.WriteInt32(g_paramInt32);
        datas.WriteString(g_paramString);
        datas.RewindRead(0);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::REPORT_WORK_SCHEDULER_STATUS),
            datas, reply, option);
    }

    void CoverageHandleGetAllowList()
    {
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option = {MessageOption::TF_SYNC};
        datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::GET_ALLOW_LIST),
            datas, reply, option);
        datas.WriteUint32(g_paramUint32);
        datas.WriteUint32(g_paramUint32);
        datas.RewindRead(0);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::GET_ALLOW_LIST),
            datas, reply, option);
    }

    void CoverageHandleSubscribeStandbyCallback()
    {
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option;
        datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
        datas.WriteRemoteObject(subscriber->AsObject());
        datas.WriteString(g_paramString);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::SUBSCRIBE_STANDBY_CALLBACK),
            datas, reply, option);
    }

    void CoverageHandleUnSubscribeStandbyCallback()
    {
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option;
        datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
        datas.WriteRemoteObject(subscriber->AsObject());
        datas.WriteString(g_paramString);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::UNSUBSCRIBE_STANDBY_CALLBACK),
            datas, reply, option);
    }

    void CoverageHandleApplyAllowResource()
    {
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option;
        datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
        resourceRequest->Marshalling(datas);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::APPLY_ALLOW_RESOURCE),
            datas, reply, option);
    }

    void CoverageUnHandleApplyAllowResource()
    {
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option;
        datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
        resourceRequest->Marshalling(datas);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::UNAPPLY_ALLOW_RESOURCE),
            datas, reply, option);
    }

    void CoverageHandleCommonEvent()
    {
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option;
        datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
        datas.WriteUint32(g_paramUint32);
        datas.WriteInt64(g_paramUint64);
        datas.WriteString(g_paramString);
        datas.RewindRead(0);
        DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(
            static_cast<uint32_t>(IStandbyInterfaceCode::HANDLE_EVENT),
            datas, reply, option);
    }

    void PreciseCoverage()
    {
        CoverageHandleIsStrategyEnabled();
        CoverageHandleReportWorkSchedulerStatus();
        CoverageHandleGetAllowList();
        CoverageHandleSubscribeStandbyCallback();
        CoverageHandleUnSubscribeStandbyCallback();
        CoverageHandleApplyAllowResource();
        CoverageUnHandleApplyAllowResource();
        CoverageHandleCommonEvent();
        if (g_initFlag) {
            return;
        }
        g_initFlag = true;
#ifdef DEVICE_STANDBY_ACCESS_TOKEN_ENABLE
        auto tokenId = Security::AccessToken::AccessTokenKit::GetNativeTokenId(RSS_NAME);
        SetSelfTokenID(tokenId);
#endif
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
    {
        g_baseFuzzData = data;
        g_baseFuzzSize = size;
        g_baseFuzzPos = 0;
        PreciseCoverage();
        for (uint32_t i = 0; i < MAX_CODE; i++) {
            MessageParcel datas;
            datas.WriteInterfaceToken(DEVICE_STANDBY_TOKEN);
            datas.WriteString(g_paramString);
            datas.WriteUint32(g_paramUint32);
            datas.WriteUint32(g_paramUint32);
            datas.WriteBuffer(data, size);
            datas.RewindRead(0);
            MessageParcel reply;
            MessageOption option;
            DelayedSingleton<StandbyService>::GetInstance()->OnRemoteRequest(i, datas, reply, option);
        }
        return true;
    }
} // namespace DevStandbyMgr
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::DevStandbyMgr::U32_AT_SIZE) {
        return 0;
    }
    OHOS::DevStandbyMgr::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
