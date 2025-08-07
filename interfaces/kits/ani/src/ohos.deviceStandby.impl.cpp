# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


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
    
}

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_getExemptedAppsSync(getExemptedAppsSync);
TH_EXPORT_CPP_API_requestExemptionResource(requestExemptionResource);
TH_EXPORT_CPP_API_releaseExemptionResource(releaseExemptionResource);