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

#include "allow_record.h"

namespace OHOS {
namespace DevStandbyMgr {
nlohmann::json AllowRecord::ParseToJson()
{
    nlohmann::json value;
    value["uid"] = uid_;
    value["pid"] = pid_;
    value["name"] = name_;
    value["allowType"] = allowType_;
    value["reasonCode"] = reasonCode_;
    if (!allowTimeList_.empty()) {
        nlohmann::json allowList;
        for (const auto &iter : allowTimeList_) {
            nlohmann::json info;
            info["allowTypeIndex"] = iter.allowTypeIndex_;
            info["endTime"] = iter.endTime_;
            info["reason"] = iter.reason_;
            allowList.push_back(info);
        }
        value["allowTimeList"] = allowList;
    }
    return value;
}

bool AllowRecord::setAllowTime(const nlohmann::json& persistTime)
{
    bool checkAllowTypeIndex = !persistTime.contains("allowTypeIndex")
        || !persistTime["allowTypeIndex"].is_number_integer();
    bool checkEndTime = !persistTime.contains("endTime") || !persistTime["endTime"].is_number_integer();
    bool checkReason = !persistTime.contains("reason") || !persistTime["reason"].is_string();
    if (checkAllowTypeIndex || checkEndTime || checkReason) {
        return false;
    }
    uint32_t allowTypeIndex = persistTime.at("allowTypeIndex").get<uint32_t>();
    int64_t endTime_ = persistTime.at("endTime").get<int64_t>();
    std::string reason_ = persistTime.at("reason").get<std::string>();
    this->allowTimeList_.emplace_back(AllowTime {allowTypeIndex, endTime_,
                reason_});
    return true;
}

bool AllowRecord::setAllowRecordField(const nlohmann::json& value)
{
    bool checkUid = !value.contains("uid") || !value["uid"].is_number_integer();
    bool checkPid = !value.contains("pid") || !value["pid"].is_number_integer();
    bool checkName = !value.contains("name") || !value["name"].is_string();
    bool checkAllowType = !value.contains("allowType") || !value["allowType"].is_number_integer();
    bool checkReasonCode = !value.contains("reasonCode") || !value["reasonCode"].is_number_integer();
    bool checkParam = checkUid || checkPid || checkName || checkAllowType || checkReasonCode;
    if (checkParam) {
        return false;
    }
    this->uid_ = value.at("uid").get<int32_t>();
    this->pid_ = value.at("pid").get<int32_t>();
    this->name_ = value.at("name").get<std::string>();
    this->allowType_ = value.at("allowType").get<uint32_t>();
    this->reasonCode_ = value.at("reasonCode").get<uint32_t>();
    return true;
}

bool AllowRecord::ParseFromJson(const nlohmann::json& value)
{
    if (value.empty() || !setAllowRecordField(value)) {
        return false;
    }
    if (value.contains("allowTimeList") && value["allowTimeList"].is_array()) {
        const nlohmann::json &allowTimeVal = value.at("allowTimeList");
        auto nums = static_cast<int32_t>(allowTimeVal.size());
        for (int i = 0; i < nums; ++i) {
            const nlohmann::json &persistTime = allowTimeVal.at(i);
            if (!setAllowTime(persistTime)) {
                return false;
            }
        }
    }
    return true;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
