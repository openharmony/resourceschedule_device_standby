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
#include "report_data_utils.h"

#include <dlfcn.h>
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    const std::string RES_SCHED_SERVICE_SO = "libresschedsvc.z.so";
}

ReportDataUtils::ReportDataUtils()
{
    LoadUtils();
}

ReportDataUtils::~ReportDataUtils()
{
    reportFunc_ = nullptr;
    dlclose(handle_);
}

ReportDataUtils& ReportDataUtils::GetInstance()
{
    static ReportDataUtils instance;
    return instance;
}

void ReportDataUtils::LoadUtils()
{
    handle_ = dlopen(RES_SCHED_SERVICE_SO.c_str(), RTLD_NOW);
    if (!handle_) {
        STANDBYSERVICE_LOGW("%{public}s load %{public}s failed!", __func__, RES_SCHED_SERVICE_SO.c_str());
        return;
    }

    reportFunc_ = reinterpret_cast<ReportDataFunc>(dlsym(handle_, "ReportDataInProcess"));
    if (!reportFunc_) {
        STANDBYSERVICE_LOGW("%{public}s load function:ReportDataInProcess failed!", __func__);
        dlclose(handle_);
        return;
    }
}

void ReportDataUtils::ReportDataInProcess(uint32_t resType, int64_t value, const nlohmann::json& payload)
{
    if (!reportFunc_) {
        STANDBYSERVICE_LOGD("%{public}s failed, function nullptr.", __func__);
        return;
    }
    reportFunc_(resType, value, payload);
}

} // namespace DevStandbyMgr
} // namespace OHOS