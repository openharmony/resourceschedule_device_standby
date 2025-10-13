/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ohos.resourceschedule.deviceStandby.proj.hpp"
#include "ohos.resourceschedule.deviceStandby.impl.hpp"
#include "taihe/runtime.hpp"
#include "taihe/optional.hpp"
#include "stdexcept"

#include "standby_service_client.h"
#include "standby_service_log.h"
#include "standby_service_errors.h"
#include "allow_type.h"

using namespace taihe;
using namespace ohos::resourceschedule::deviceStandby;
using namespace OHOS;
using namespace OHOS::DevStandbyMgr;

namespace {

int32_t FindErrCode(const int32_t errCodeIn)
{
    auto iter = paramErrCodeMsgMap.find(errCodeIn);
    if (iter != paramErrCodeMsgMap.end()) {
        return ERR_STANDBY_INVALID_PARAM;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}

std::string FindErrMsg(const int32_t errCode)
{
    if (errCode == ERR_OK) {
        return "";
    }
    auto iter = saErrCodeMsgMap.find(errCode);
    if (iter != saErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        int32_t errCodeInfo = FindErrCode(errCode);
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

void HandleErrCode(int32_t errCode)
{
    STANDBYSERVICE_LOGI("HandleErrCode errCode = %{public}d", errCode);
    if (errCode == ERR_OK) {
        return;
    }
    std::string errMsg = FindErrMsg(errCode);
    int32_t errCodeInfo = FindErrCode(errCode);
    if (errMsg != "") {
        ::taihe::set_business_error(errCodeInfo, errMsg);
    }
}

void HandleParamErr(int32_t errCode)
{
    STANDBYSERVICE_LOGI("HandleParamErr errCode = %{public}d, isThrow = true", errCode);
    if (errCode == ERR_OK) {
        return;
    }
    auto iter = paramErrCodeMsgMap.find(errCode);
    if (iter != paramErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(paramErrCodeMsgMap[errCode]);
        ::taihe::set_business_error(ERR_STANDBY_INVALID_PARAM, errMessage);
    }
}

::taihe::optional<::ohos::resourceschedule::deviceStandby::ExemptedAppInfo> GenAniExemptedAppInfo(
    const OHOS::DevStandbyMgr::AllowInfo& allowInfo)
{
    ::ohos::resourceschedule::deviceStandby::ExemptedAppInfo exemptedAppInfo {
        .resourceTypes = allowInfo.GetAllowType(),
        .name = std::string((allowInfo.GetName()).c_str()),
        .duration = allowInfo.GetDuration()
    };
    return ::taihe::optional<::ohos::resourceschedule::deviceStandby::ExemptedAppInfo>::make(exemptedAppInfo);
}

sptr<OHOS::DevStandbyMgr::ResourceRequest> GenResourceRequest(
    ::ohos::resourceschedule::deviceStandby::ResourceRequest const& request)
{
    return new (std::nothrow) OHOS::DevStandbyMgr::ResourceRequest(request.resourceTypes, request.uid,
        std::string((request.name).c_str()), request.duration,
        std::string((request.reason).c_str()), OHOS::DevStandbyMgr::ReasonCodeEnum::REASON_APP_API);
}

bool VerifyAniResourceRequest(::ohos::resourceschedule::deviceStandby::ResourceRequest const& request)
{
    if (request.resourceTypes <= 0
        || static_cast<uint32_t>(request.resourceTypes) > OHOS::DevStandbyMgr::MAX_ALLOW_TYPE_NUMBER) {
        HandleParamErr(ERR_RESOURCE_TYPES_INVALID);
        return false;
    }
    if (request.uid < 0) {
        HandleParamErr(ERR_UID_INVALID);
        return false;
    }
    if (request.name.empty()) {
        HandleParamErr(ERR_NAME_INVALID_OR_EMPTY);
        return false;
    }
    return true;
}

::taihe::array<::ohos::resourceschedule::deviceStandby::ExemptedAppInfo> GetExemptedAppsSync(int32_t resourceTypes)
{
    std::vector<AllowInfo> allowInfoArray {};
    int32_t ret = StandbyServiceClient::GetInstance().GetAllowList(
        resourceTypes, allowInfoArray, ReasonCodeEnum::REASON_APP_API);
    std::vector<::ohos::resourceschedule::deviceStandby::ExemptedAppInfo> allowLists;
    if (ret != ERR_OK) {
        HandleErrCode(ret);
        return ::taihe::array<::ohos::resourceschedule::deviceStandby::ExemptedAppInfo>(allowLists);
    }
    for (const auto& allowInfo : allowInfoArray) {
        auto result = GenAniExemptedAppInfo(allowInfo);
        if (result.has_value()) {
            allowLists.push_back(result.value());
        }
    }
    return ::taihe::array<::ohos::resourceschedule::deviceStandby::ExemptedAppInfo>(allowLists);
}

void RequestExemptionResource(::ohos::resourceschedule::deviceStandby::ResourceRequest const& request)
{
    VerifyAniResourceRequest(request);
    sptr<OHOS::DevStandbyMgr::ResourceRequest> resourceRequest = GenResourceRequest(request);
    int32_t ret = StandbyServiceClient::GetInstance().ApplyAllowResource(resourceRequest);
    if (ret != ERR_OK) {
        HandleErrCode(ret);
    }
}

void ReleaseExemptionResource(::ohos::resourceschedule::deviceStandby::ResourceRequest const& request)
{
    VerifyAniResourceRequest(request);
    sptr<OHOS::DevStandbyMgr::ResourceRequest> resourceRequest = GenResourceRequest(request);
    int32_t ret = StandbyServiceClient::GetInstance().UnapplyAllowResource(resourceRequest);
    if (ret != ERR_OK) {
        HandleErrCode(ret);
    }
}

bool IsDeviceInStandby()
{
    bool isStandby;
    int32_t ret = StandbyServiceClient::GetInstance().IsDeviceInStandby(isStandby);
    if (ret != ERR_OK) {
        HandleErrCode(ret);
        return false;
    }
    return isStandby;
}
} // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_getExemptedAppsSync(GetExemptedAppsSync);
TH_EXPORT_CPP_API_requestExemptionResource(RequestExemptionResource);
TH_EXPORT_CPP_API_releaseExemptionResource(ReleaseExemptionResource);
TH_EXPORT_CPP_API_isDeviceInStandby(IsDeviceInStandby);
// NOLINTEND