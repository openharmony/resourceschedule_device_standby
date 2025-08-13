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

#include "common.h"

#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
const uint32_t ASYNC_CALLBACK_PARAM_NUM = 2;
const uint32_t STRING_MAX_SIZE = 128;
}

AsyncWorkData::AsyncWorkData(napi_env napiEnv)
{
    env = napiEnv;
}

AsyncWorkData::~AsyncWorkData()
{
    if (callback) {
        STANDBYSERVICE_LOGD("AsyncWorkData::~AsyncWorkData delete callback");
        napi_delete_reference(env, callback);
        callback = nullptr;
    }
    if (asyncWork) {
        STANDBYSERVICE_LOGD("AsyncWorkData::~AsyncWorkData delete asyncWork");
        napi_delete_async_work(env, asyncWork);
        asyncWork = nullptr;
    }
}

napi_value Common::NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

void Common::PaddingAsyncWorkData(
    const napi_env& env, const napi_ref& callback, AsyncWorkData& info, napi_value& promise)
{
    if (callback) {
        info.callback = callback;
        info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        info.deferred = deferred;
        info.isCallback = false;
    }
}

void Common::ReturnCallbackPromise(const napi_env& env, const AsyncWorkData& info, const napi_value& result)
{
    if (info.isCallback) {
        SetCallback(env, info.callback, info.errCode, result);
    } else {
        SetPromise(env, info, result);
    }
}

void Common::SetCallback(
    const napi_env& env, const napi_ref& callbackIn, const int32_t& errCode, const napi_value& result)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[ASYNC_CALLBACK_PARAM_NUM] = {nullptr};
    if (errCode == ERR_OK) {
        results[0] = NapiGetNull(env);
    } else {
        int32_t errCodeInfo = FindErrCode(env, errCode);
        std::string errMsg = FindErrMsg(env, errCode);
        results[0] = GetCallbackErrorValue(env, errCodeInfo, errMsg);
    }
    results[1] = result;
    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, undefined, callback, ASYNC_CALLBACK_PARAM_NUM, &results[0], &resultout));
}

napi_value Common::SetPromise(
    const napi_env& env, const AsyncWorkData& info, const napi_value& result)
{
    if (info.errCode == ERR_OK) {
        napi_resolve_deferred(env, info.deferred, result);
    } else {
        int32_t errCodeInfo = FindErrCode(env, info.errCode);
        std::string errMsg = FindErrMsg(env, info.errCode);
        napi_value error = nullptr;
        napi_value eCode = nullptr;
        napi_value eMsg = nullptr;
        NAPI_CALL(env, napi_create_int32(env, errCodeInfo, &eCode));
        NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(),
            errMsg.length(), &eMsg));
        NAPI_CALL(env, napi_create_object(env, &error));
        NAPI_CALL(env, napi_set_named_property(env, error, "data", eCode));
        NAPI_CALL(env, napi_set_named_property(env, error, "code", eCode));
        NAPI_CALL(env, napi_set_named_property(env, error, "message", eMsg));
        napi_reject_deferred(env, info.deferred, error);
    }
    return result;
}

napi_value Common::GetCallbackErrorValue(napi_env env, const int32_t errCode, const std::string errMsg)
{
    if (errCode == ERR_OK) {
        return NapiGetNull(env);
    }
    napi_value error = nullptr;
    napi_value eCode = nullptr;
    napi_value eMsg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(),
        errMsg.length(), &eMsg));
    NAPI_CALL(env, napi_create_object(env, &error));
    NAPI_CALL(env, napi_set_named_property(env, error, "code", eCode));
    NAPI_CALL(env, napi_set_named_property(env, error, "message", eMsg));
    return error;
}

void Common::HandleErrCode(const napi_env &env, int32_t errCode)
{
    if (errCode == ERR_OK) {
        return;
    }
    STANDBYSERVICE_LOGI("HandleErrCode errCode = %{public}d", errCode);
    std::string errMsg = FindErrMsg(env, errCode);
    int32_t errCodeInfo = FindErrCode(env, errCode);
    if (errMsg != "") {
        napi_throw_error(env, std::to_string(errCodeInfo).c_str(), errMsg.c_str());
    }
}

std::string Common::FindErrMsg(const napi_env& env, const int32_t errCode)
{
    if (errCode == ERR_OK) {
        return "";
    }
    auto iter = saErrCodeMsgMap.find(errCode);
    if (iter != saErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        int32_t errCodeInfo = FindErrCode(env, errCode);
        errMessage.append(std::to_string(errCodeInfo)).append(": ").append(iter->second);
        return errMessage;
    }
    iter = paramErrCodeMsgMap.find(errCode);
    if (iter != paramErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(paramErrCodeMsgMap[errCode]);
        return errMessage;
    }
    return "Inner error.";
}

int32_t Common::FindErrCode(const napi_env& env, const int32_t errCodeIn)
{
    auto iter = paramErrCodeMsgMap.find(errCodeIn);
    if (iter != paramErrCodeMsgMap.end()) {
        return ERR_STANDBY_INVALID_PARAM;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}

bool Common::HandleParamErr(const napi_env& env, int32_t errCode)
{
    STANDBYSERVICE_LOGI("HandleParamErr errCode = %{public}d, isThrow = true", errCode);
    if (errCode == ERR_OK) {
        return false;
    }
    auto iter = paramErrCodeMsgMap.find(errCode);
    if (iter != paramErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(paramErrCodeMsgMap[errCode]);
        napi_throw_error(env, std::to_string(ERR_STANDBY_INVALID_PARAM).c_str(), errMessage.c_str());
        return true;
    }
    return false;
}

void Common::SetInt32Value(const napi_env& env, const std::string& fieldStr, const int32_t intValue, napi_value& result)
{
    napi_value value = nullptr;
    napi_create_int32(env, intValue, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

void Common::SetUint32Value(const napi_env& env, const std::string& fieldStr,
    const uint32_t uintValue, napi_value& result)
{
    napi_value value = nullptr;
    napi_create_uint32(env, uintValue, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

napi_value Common::GetStringValue(const napi_env &env, const napi_value &value, std::string &result)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_string) {
        return nullptr;
    }

    char str[STRING_MAX_SIZE] = {0};
    size_t strLen = 0;
    napi_status status = napi_get_value_string_utf8(env, value, str, STRING_MAX_SIZE - 1, &strLen);
    if (status != napi_ok) {
        return nullptr;
    }
    result = std::string(str);
    STANDBYSERVICE_LOGD("GetStringValue result: %{public}s", result.c_str());
    return Common::NapiGetNull(env);
}

void Common::SetStringValue(const napi_env& env, const std::string& fieldStr,
    const std::string& stringValue, napi_value& result)
{
    napi_value value = nullptr;
    napi_create_string_utf8(env, stringValue.c_str(), stringValue.length(), &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

napi_value Common::GetUint32Value(const napi_env& env, const napi_value& value, uint32_t& result)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_number) {
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_uint32(env, value, &result));
    STANDBYSERVICE_LOGI("GetUint32Value result: %{public}d", static_cast<int32_t>(result));

    return Common::NapiGetNull(env);
}

napi_value Common::GetInt32Value(const napi_env& env, const napi_value& value, int32_t& result)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_number) {
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_int32(env, value, &result));
    STANDBYSERVICE_LOGD("GetInt32Value result: %{public}d", static_cast<int32_t>(result));

    return Common::NapiGetNull(env);
}

napi_value Common::GetNamedInt32Value(const napi_env &env, napi_value &object, const char* utf8name,
    int32_t& result)
{
    bool hasNamedProperty = false;
    napi_value intValue = nullptr;
    if (napi_has_named_property(env, object, utf8name, &hasNamedProperty) != napi_ok || !hasNamedProperty) {
        STANDBYSERVICE_LOGE("GetNamedInt32Value failed, %{public}s not exist, is nullptr", utf8name);
        return nullptr;
    }
    NAPI_CALL(env, napi_get_named_property(env, object, utf8name, &intValue));
    if (!Common::GetInt32Value(env, intValue, result)) {
        STANDBYSERVICE_LOGE("GetNamedInt32Value failed, %{public}s is nullptr", utf8name);
        return nullptr;
    }
    STANDBYSERVICE_LOGD("GetNamedInt32Value: %{public}s is %{public}d", utf8name, result);
    return Common::NapiGetNull(env);
}

napi_value Common::GetNamedStringValue(const napi_env &env, napi_value &object, const char* utf8name,
    std::string& result)
{
    bool hasNamedProperty = false;
    napi_value stringValue = nullptr;
    if (napi_has_named_property(env, object, utf8name, &hasNamedProperty) != napi_ok || !hasNamedProperty) {
        STANDBYSERVICE_LOGE("GetNamedStringValue failed, string not exist, is nullptr");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_named_property(env, object, utf8name, &stringValue));
    if (!Common::GetStringValue(env, stringValue, result)) {
        STANDBYSERVICE_LOGE("GetStringValue failed, the string is nullptr");
        return nullptr;
    }
    STANDBYSERVICE_LOGD("GetNamedStringValue: result is %{public}s", result.c_str());
    return Common::NapiGetNull(env);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS