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

#ifndef STANDBY_REPORT_DATA_UTILS
#define STANDBY_REPORT_DATA_UTILS

#include "nlohmann/json.hpp"

namespace OHOS {
namespace DevStandbyMgr {
using ReportDataFunc = void (*)(uint32_t resType, int64_t value, const nlohmann::json& payload);
class ReportDataUtils {
public:
    ReportDataUtils();
    ~ReportDataUtils();
    static ReportDataUtils& GetInstance();
    void ReportDataInProcess(uint32_t resType, int64_t value, const nlohmann::json& payload);

private:
    void LoadUtils();

    ReportDataFunc reportFunc_ = nullptr;
    void *handle_ = nullptr;
};
} // namespace DevStandbyMgr
} // namespace OHOS
#endif // STANDBY_REPORT_DATA_UTILS