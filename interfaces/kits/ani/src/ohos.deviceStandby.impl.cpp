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
#include "ohos.resourceschedule.deviceStandby.hpp"
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

std::string HandleParamErr(int32_t errCode)
{
    STANDBYSERVICE_LOGI("ani HandleParamErr errCode = %{public}d, isThrow = true", errCode);
    if (errCode == ERR_OK) {
        return "";
    }
    auto iter = saErrCodeMsgMap.find(errCode);
    if (iter != saErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        errMessage.append(": ").append(iter->second);
        return errMessage;
    }
    iter = paramErrCodeMsgMap.find(errCode);
    if (iter != paramErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(iter->second);
        return errMessage;
    }
    return "Inner error.";
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
    int32_t errCode = 0;
    if (request.resourceTypes <= 0
        || static_cast<uint32_t>(request.resourceTypes) > OHOS::DevStandbyMgr::MAX_ALLOW_TYPE_NUMBER) {
        errCode = ERR_RESOURCE_TYPES_INVALID;
        ::taihe::set_business_error(errCode, HandleParamErr(errCode));
        return false;
    }
    if (request.uid < 0) {
        errCode = ERR_UID_INVALID;
        ::taihe::set_business_error(errCode, HandleParamErr(errCode));
        return false;
    }
    if (request.name.empty()) {
        errCode = ERR_NAME_INVALID_OR_EMPTY;
        ::taihe::set_business_error(errCode, HandleParamErr(errCode));
        return false;
    }
    return true;
}

::taihe::array<::ohos::resourceschedule::deviceStandby::ExemptedAppInfo> getExemptedAppsSync(int32_t resourceTypes)
{
    std::vector<AllowInfo> allowInfoArray {};
    int32_t ret = StandbyServiceClient::GetInstance().GetAllowList(
        resourceTypes, allowInfoArray, ReasonCodeEnum::REASON_APP_API);
    std::vector<::ohos::resourceschedule::deviceStandby::ExemptedAppInfo> allowLists;
    if (ret != ERR_OK) {
        ::taihe::set_business_error(ret, HandleParamErr(ret));
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

void requestExemptionResource(::ohos::resourceschedule::deviceStandby::ResourceRequest const& request)
{
    VerifyAniResourceRequest(request);
    sptr<OHOS::DevStandbyMgr::ResourceRequest> resourceRequest = GenResourceRequest(request);
    int32_t ret = StandbyServiceClient::GetInstance().ApplyAllowResource(resourceRequest);
    if (ret != ERR_OK) {
        ::taihe::set_business_error(ret, HandleParamErr(ret));
    }
}

void releaseExemptionResource(::ohos::resourceschedule::deviceStandby::ResourceRequest const& request)
{
    VerifyAniResourceRequest(request);
    sptr<OHOS::DevStandbyMgr::ResourceRequest> resourceRequest = GenResourceRequest(request);
    int32_t ret = StandbyServiceClient::GetInstance().UnapplyAllowResource(resourceRequest);
    if (ret != ERR_OK) {
        ::taihe::set_business_error(ret, HandleParamErr(ret));
    }
}

bool isDeviceInStandby()
{
    bool isStandby;
    int32_t ret = StandbyServiceClient::GetInstance().IsDeviceInStandby(isStandby);
    if (ret != ERR_OK) {
        ::taihe::set_business_error(ret, HandleParamErr(ret));
        return false;
    }
    return isStandby;
}
} // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_getExemptedAppsSync(getExemptedAppsSync);
TH_EXPORT_CPP_API_requestExemptionResource(requestExemptionResource);
TH_EXPORT_CPP_API_releaseExemptionResource(releaseExemptionResource);
TH_EXPORT_CPP_API_isDeviceInStandby(isDeviceInStandby);
// NOLINTEND