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

#include "standby_napi_module.h"

#include <vector>

#include "napi_base_context.h"
#include "singleton.h"

#include "allow_info.h"
#include "allow_type.h"
#include "resourcce_request.h"
#include "standby_service_log.h"
#include "standby_service_client.h"

#include "common.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
constexpr uint32_t IS_DEVICE_IN_STANDBY_MIN_PARAMS = 0;
constexpr uint32_t IS_DEVICE_IN_STANDBY_PARAMS = 1;
constexpr uint32_t GET_ALLOW_LIST_MIN_PARAMS = 1;
constexpr uint32_t GET_ALLOW_LIST_STANDBY_PARAMS = 2;
constexpr uint32_t APPLY_ALLOW_RESOURCE_PARAMS = 1;
}

struct AsyncCallbackInfoIsDeviceInStandby : public AsyncWorkData {
    explicit AsyncCallbackInfoIsDeviceInStandby(napi_env env) : AsyncWorkData(env) {}
    bool isStandby {false};
};

struct IsDeviceInStandbyParamsInfo {
    napi_ref callback = nullptr;
};

napi_value ParseInStandbyParameters(const napi_env& env, const napi_callback_info& info,
    IsDeviceInStandbyParamsInfo& params)
{
    size_t argc = IS_DEVICE_IN_STANDBY_PARAMS;
    napi_value argv[IS_DEVICE_IN_STANDBY_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != IS_DEVICE_IN_STANDBY_MIN_PARAMS && argc != IS_DEVICE_IN_STANDBY_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR);
        return nullptr;
    }

    // argv[0]: callback
    if (argc == IS_DEVICE_IN_STANDBY_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        if (valuetype != napi_function) {
            Common::HandleParamErr(env, ERR_CALLBACK_NULL_OR_TYPE_ERR);
            return nullptr;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[0], 1, &params.callback));
    }

    return Common::NapiGetNull(env);
}

void AddInStandbyExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfoIsDeviceInStandby *asyncCallbackInfo =
    static_cast<AsyncCallbackInfoIsDeviceInStandby *>(data);
    if (asyncCallbackInfo != nullptr) {
        // return current state, whether in standby state or not
        asyncCallbackInfo->errCode = StandbyServiceClient::GetInstance().
            IsDeviceInStandby(asyncCallbackInfo->isStandby);
    }
}

void AddInStandbyCallbackCompleteCB(napi_env env, napi_status status, void *data)
{
    AsyncCallbackInfoIsDeviceInStandby *asyncCallbackInfo =
        static_cast<AsyncCallbackInfoIsDeviceInStandby *>(data);
    std::unique_ptr<AsyncCallbackInfoIsDeviceInStandby> callbackPtr {asyncCallbackInfo};
    if (asyncCallbackInfo != nullptr) {
        napi_value result = nullptr;
        napi_create_int32(env, asyncCallbackInfo->isStandby, &result);
        Common::ReturnCallbackPromise(env, *asyncCallbackInfo, result);
    }
}

napi_value IsDeviceInStandby(napi_env env, napi_callback_info info)
{
    IsDeviceInStandbyParamsInfo params;
    if (ParseInStandbyParameters(env, info, params) == nullptr) {
        return nullptr;
    }

    napi_value promise = nullptr;
    AsyncCallbackInfoIsDeviceInStandby *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoIsDeviceInStandby(env);
    if (!asyncCallbackInfo) {
        return nullptr;
    }
    std::unique_ptr<AsyncCallbackInfoIsDeviceInStandby> callbackPtr {asyncCallbackInfo};
    Common::PaddingAsyncWorkData(env, params.callback, *asyncCallbackInfo, promise);

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, "IsDeviceInStandby", NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, AddInStandbyExecuteCB,
        AddInStandbyCallbackCompleteCB, static_cast<void *>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    callbackPtr.release();
    if (asyncCallbackInfo->isCallback) {
        return Common::NapiGetNull(env);
    }
    return promise;
}

struct AsyncCallbackInfoGetAllowList : public AsyncWorkData {
    explicit AsyncCallbackInfoGetAllowList(napi_env env) : AsyncWorkData(env) {}
    uint32_t allowType = 0;
    std::vector<AllowInfo> allowInfoArray {};
};

struct GetAllowListParamsInfo {
    uint32_t allowType = 0;
    napi_ref callback = nullptr;
};

void AddGetAllowListExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfoGetAllowList *asyncCallbackInfo =
    static_cast<AsyncCallbackInfoGetAllowList *>(data);
    if (asyncCallbackInfo != nullptr) {
        asyncCallbackInfo->errCode = StandbyServiceClient::GetInstance().GetAllowList(
            asyncCallbackInfo->allowType, asyncCallbackInfo->allowInfoArray, ReasonCodeEnum::REASON_APP_API);
    }
}

void AddGetAllowListCallbackCompleteCB(napi_env env, napi_status status, void *data)
{
    AsyncCallbackInfoGetAllowList *asyncCallbackInfo =
        static_cast<AsyncCallbackInfoGetAllowList *>(data);
    std::unique_ptr<AsyncCallbackInfoGetAllowList> callbackPtr {asyncCallbackInfo};
    if (asyncCallbackInfo != nullptr) {
        napi_value result = nullptr;
        napi_create_array(env, &result);
        int32_t index = 0;
        for (const auto& allowInfo : asyncCallbackInfo->allowInfoArray) {
            napi_value allowInfoObj = nullptr;
            napi_create_object(env, &allowInfoObj);
            Common::SetUint32Value(env, "resourceTypes", static_cast<uint32_t>(allowInfo.GetAllowType()), allowInfoObj);
            Common::SetStringValue(env, "name", allowInfo.GetName(), allowInfoObj);
            Common::SetInt32Value(env, "duration", allowInfo.GetDuration(), allowInfoObj);
            napi_set_element(env, result, index, allowInfoObj);
            ++index;
        }

        Common::ReturnCallbackPromise(env, *asyncCallbackInfo, result);
    }
}

napi_value ParseGetAllowListParameters(const napi_env& env, const napi_callback_info& info,
    GetAllowListParamsInfo& params)
{
    size_t argc = GET_ALLOW_LIST_STANDBY_PARAMS;
    napi_value argv[GET_ALLOW_LIST_STANDBY_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != GET_ALLOW_LIST_MIN_PARAMS && argc != GET_ALLOW_LIST_STANDBY_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR);
        return nullptr;
    }

    // argv[0]: allowType
    if (Common::GetUint32Value(env, argv[0], params.allowType) == nullptr) {
        STANDBYSERVICE_LOGE("ParseParameters failed, allowType is nullptr.");
        Common::HandleParamErr(env, ERR_RESOURCE_TYPES_INVALID);
        return nullptr;
    }

    // argv[1]: callback
    if (argc == GET_ALLOW_LIST_STANDBY_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            Common::HandleParamErr(env, ERR_CALLBACK_NULL_OR_TYPE_ERR);
            return nullptr;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &params.callback));
    }

    if (params.allowType == 0) {
        Common::HandleParamErr(env, ERR_RESOURCE_TYPES_INVALID);
        return nullptr;
    }

    return Common::NapiGetNull(env);
}

napi_value GetExemptionListApps(napi_env env, napi_callback_info info)
{
    GetAllowListParamsInfo params;
    if (ParseGetAllowListParameters(env, info, params) == nullptr) {
        return nullptr;
    }

    napi_value promise = nullptr;
    AsyncCallbackInfoGetAllowList *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoGetAllowList(env);
    if (!asyncCallbackInfo) {
        return nullptr;
    }
    std::unique_ptr<AsyncCallbackInfoGetAllowList> callbackPtr {asyncCallbackInfo};
    asyncCallbackInfo->allowType = params.allowType;
    Common::PaddingAsyncWorkData(env, params.callback, *asyncCallbackInfo, promise);

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, "GetAllowList", NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, AddGetAllowListExecuteCB,
        AddGetAllowListCallbackCompleteCB, static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    callbackPtr.release();
    if (asyncCallbackInfo->isCallback) {
        return Common::NapiGetNull(env);
    }
    return promise;
}

napi_value ParseAllowResParameters(const napi_env &env, const napi_callback_info &info,
    sptr<ResourceRequest> &resourceRequest)
{
    size_t argc = APPLY_ALLOW_RESOURCE_PARAMS;
    napi_value argv[APPLY_ALLOW_RESOURCE_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != APPLY_ALLOW_RESOURCE_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR);
        return nullptr;
    }
    int32_t allowType {0};
    int32_t uid {0};
    std::string name {};
    int32_t duration {0};
    std::string reason {};

    if (!Common::GetNamedInt32Value(env, argv[0], "resourceTypes", allowType) || allowType <= 0
        || static_cast<uint32_t>(allowType) > MAX_ALLOW_TYPE_NUMBER) {
        Common::HandleParamErr(env, ERR_RESOURCE_TYPES_INVALID);
        return nullptr;
    }
    if (!Common::GetNamedInt32Value(env, argv[0], "uid", uid) || uid < 0) {
        Common::HandleParamErr(env, ERR_UID_INVALID);
        return nullptr;
    }
    if (!Common::GetNamedStringValue(env, argv[0], "name", name) || name.empty()) {
        Common::HandleParamErr(env, ERR_NAME_INVALID_OR_EMPTY);
        return nullptr;
    }
    if (!Common::GetNamedInt32Value(env, argv[0], "duration", duration)) {
        Common::HandleParamErr(env, ERR_DURATION_INVALID);
        return nullptr;
    }
    if (!Common::GetNamedStringValue(env, argv[0], "reason", reason)) {
        Common::HandleParamErr(env, ERR_REASON_INVALID_TYPE_ERR);
        return nullptr;
    }
    resourceRequest = new (std::nothrow) ResourceRequest(allowType, uid,
        name, duration, reason, ReasonCodeEnum::REASON_APP_API);
    return Common::NapiGetNull(env);
}

napi_value ApplyAllowResource(napi_env env, napi_callback_info info)
{
    sptr<ResourceRequest> resourceRequest {};
    if (ParseAllowResParameters(env, info, resourceRequest) == nullptr) {
        return Common::NapiGetNull(env);
    }
    ErrCode errCode = StandbyServiceClient::GetInstance().ApplyAllowResource(resourceRequest);
    Common::HandleErrCode(env, errCode);
    return Common::NapiGetNull(env);
}

napi_value UnapplyAllowResource(napi_env env, napi_callback_info info)
{
    sptr<ResourceRequest> resourceRequest {};
    if (ParseAllowResParameters(env, info, resourceRequest) == nullptr) {
        return Common::NapiGetNull(env);
    }
    ErrCode errCode = StandbyServiceClient::GetInstance().UnapplyAllowResource(resourceRequest);
    Common::HandleErrCode(env, errCode);
    return Common::NapiGetNull(env);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS