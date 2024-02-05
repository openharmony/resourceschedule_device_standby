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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_POLICY_INCLUDE_JSON_UTILS_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_POLICY_INCLUDE_JSON_UTILS_H

#include <string>

#include "nlohmann/json.hpp"
#include "standby_service_errors.h"

namespace OHOS {
namespace DevStandbyMgr {
class JsonUtils {
public:
    JsonUtils() = delete;

    /**
     * @brief load json value from file
     *
     * @param jsonValue json value from the file content
     * @param filePath the absolute file path
     * @return true true if succeed
     */
    static bool LoadJsonValueFromFile(nlohmann::json& jsonValue, const std::string& filePath);

    /**
     * @brief dump json value to file
     *
     * @param jsonValue json value to be dumped to the file
     * @param filePath the absolute file path to dump json value
     * @return true true true if succeed
     */
    static bool DumpJsonValueToFile(const nlohmann::json& jsonValue, const std::string& filePath);

    /**
     * @brief get the 32-bit int from json value object
     *
     * @param jsonValue json object
     * @param key the name of int object
     * @param value value of the int object
     * @return true true if succeed
     */
    static bool GetInt32FromJsonValue(const nlohmann::json& jsonValue, const std::string& key, int32_t& value);

    /**
     * @brief get the bool from json value object
     *
     * @param jsonValue json object
     * @param key the name of bool object
     * @param value value of the bool object
     * @return true if succeed
     */
    static bool GetBoolFromJsonValue(const nlohmann::json& jsonValue, const std::string& key, bool& value);

    /**
     * @brief get the string from json value object
     *
     * @param jsonValue json object
     * @param key the name of string object
     * @param value value of the string object
     * @return true if succeed
     */
    static bool GetStringFromJsonValue(const nlohmann::json& jsonValue, const std::string& key, std::string& value);

    /**
     * @brief Get the Obj from json value object
     *
     * @param jsonValue json object
     * @param key the name of json object
     * @param value c
     * @return true if succeed
     */
    static bool GetObjFromJsonValue(const nlohmann::json& jsonValue, const std::string& key, nlohmann::json& value);

    /**
     * @brief Get the Array from json value object
     *
     * @param jsonValue json object
     * @param key the name of array object
     * @param value value of array object
     * @return true if succeed
     */
    static bool GetArrayFromJsonValue(const nlohmann::json& jsonValue, const std::string& key, nlohmann::json& value);

    /**
     * @brief Get the String Array from json value object
     *
     * @param jsonValue json object
     * @param key the name of string array object
     * @param strArray string array
     * @return true if succeed
     */
    static bool GetStrArrFromJsonValue(const nlohmann::json& jsonValue, const std::string& key,
        std::vector<std::string>& strArray);
    static bool GetRealPath(const std::string& partialPath, std::string& fullPath);
private:
    static bool CreateNodeFile(const std::string &filePath);
    static bool GetFileContent(const std::string& filePath, std::string& content);
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_POLICY_INCLUDE_JSON_UTILS_H
