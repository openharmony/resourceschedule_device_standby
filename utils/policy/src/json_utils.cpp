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

#include "json_utils.h"

#include <climits>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    constexpr uint32_t JSON_FORMAT = 4;
}

bool JsonUtils::LoadJsonValueFromFile(nlohmann::json& jsonValue, const std::string& filePath)
{
    std::string content;
    if (!GetFileContent(filePath, content)) {
        STANDBYSERVICE_LOGE("failed to load content from %{public}s", filePath.c_str());
        return false;
    }
    if (content.empty()) {
        STANDBYSERVICE_LOGE("content of %{public}s is empty", filePath.c_str());
        return false;
    }
    jsonValue = nlohmann::json::parse(content, nullptr, false);
    if (jsonValue.is_discarded()) {
        STANDBYSERVICE_LOGE("failed to parse content from %{public}s", filePath.c_str());
        return false;
    }
    if (!jsonValue.is_object()) {
        STANDBYSERVICE_LOGE("the content of %{public}s is not an object ", filePath.c_str());
        return false;
    }
    return true;
}

bool JsonUtils::DumpJsonValueToFile(const nlohmann::json& jsonValue, const std::string& filePath)
{
    if (!CreateNodeFile(filePath)) {
        STANDBYSERVICE_LOGE("create file failed.");
        return false;
    }
    std::ofstream fout;
    std::string realPath;
    if (!GetRealPath(filePath, realPath)) {
        STANDBYSERVICE_LOGE("get real file path: %{public}s failed", filePath.c_str());
        return false;
    }
    fout.open(realPath, std::ios::out);
    if (!fout.is_open()) {
        STANDBYSERVICE_LOGE("open file: %{public}s failed.", filePath.c_str());
        return false;
    }
    fout << jsonValue.dump(JSON_FORMAT).c_str() << std::endl;
    fout.close();
    return true;
}

bool JsonUtils::CreateNodeFile(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) == ERR_OK) {
        STANDBYSERVICE_LOGD("the standby service config file: %{public}s already exists.", filePath.c_str());
        return true;
    }
    std::string fullpath {""};
    if (!GetRealPath(filePath, fullpath)) {
        STANDBYSERVICE_LOGD("the standby service config file: %{public}s not exists.", filePath.c_str());
        fullpath = filePath;
    }
    int32_t fd = open(fullpath.c_str(), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < ERR_OK) {
        STANDBYSERVICE_LOGE("Fail to open file: %{public}s", fullpath.c_str());
        return false;
    }
    close(fd);
    return true;
}

bool JsonUtils::GetInt32FromJsonValue(const nlohmann::json& jsonValue, const std::string& key, int32_t& value)
{
    if (jsonValue.empty() || key.empty()) {
        return false;
    }
    if (jsonValue.contains(key) == 0 || !jsonValue.at(key).is_number_integer()) {
        return false;
    }
    value =  jsonValue.at(key).get<int32_t>();
    return true;
}

bool JsonUtils::GetBoolFromJsonValue(const nlohmann::json& jsonValue, const std::string& key, bool& value)
{
    if (jsonValue.empty() || key.empty()) {
        return false;
    }
    if (jsonValue.contains(key) == 0 || !jsonValue.at(key).is_boolean()) {
        return false;
    }
    value =  jsonValue.at(key).get<bool>();
    return true;
}

bool JsonUtils::GetStringFromJsonValue(const nlohmann::json& jsonValue, const std::string& key, std::string& value)
{
    if (jsonValue.empty() || key.empty()) {
        return false;
    }
    if (jsonValue.contains(key) == 0 || !jsonValue.at(key).is_string()) {
        return false;
    }
    value =  jsonValue.at(key).get<std::string>();
    return true;
}

bool JsonUtils::GetObjFromJsonValue(const nlohmann::json& jsonValue, const std::string& key, nlohmann::json& value)
{
    if (jsonValue.empty() || key.empty()) {
        return false;
    }
    if (jsonValue.contains(key) == 0 || !jsonValue.at(key).is_object()) {
        return false;
    }
    value =  jsonValue.at(key);
    return true;
}

bool JsonUtils::GetArrayFromJsonValue(const nlohmann::json& jsonValue, const std::string& key, nlohmann::json& value)
{
    if (jsonValue.empty() || key.empty()) {
        return false;
    }
    if (jsonValue.contains(key) == 0 || !jsonValue.at(key).is_array()) {
        return false;
    }
    value =  jsonValue.at(key);
    return true;
}

bool JsonUtils::GetStrArrFromJsonValue(const nlohmann::json& jsonValue, const std::string& key,
    std::vector<std::string>& strArray)
{
    nlohmann::json strArrayValue;
    if (!JsonUtils::GetArrayFromJsonValue(jsonValue, key, strArrayValue)) {
        return false;
    }
    strArray.clear();
    for (const auto &strItem : strArrayValue) {
        if (!strItem.is_string()) {
            return false;
        }
        strArray.emplace_back(strItem.get<std::string>());
    }
    return true;
}

bool JsonUtils::GetFileContent(const std::string& filePath, std::string& content)
{
    std::string fullPath;
    if (!GetRealPath(filePath, fullPath)) {
        return false;
    }
    STANDBYSERVICE_LOGD("full path of standby service config file is: %{public}s ", fullPath.c_str());
    std::ifstream fin(fullPath);
    if (!fin.is_open()) {
        return false;
    }
    std::ostringstream ss;
    ss << fin.rdbuf();
    content = ss.str();
    return true;
}

bool JsonUtils::GetRealPath(const std::string& partialPath, std::string& fullPath)
{
    char tmpPath[PATH_MAX] = {0};
    if (partialPath.size() > PATH_MAX || !realpath(partialPath.c_str(), tmpPath)) {
        return false;
    }
    fullPath = tmpPath;
    return true;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS