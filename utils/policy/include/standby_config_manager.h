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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_POLICY_INCLUDE_STANDBY_CONFIG_MANAGER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_POLICY_INCLUDE_STANDBY_CONFIG_MANAGER_H

#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>


#include "json_utils.h"
#include "single_instance.h"
#include "standby_service_errors.h"
namespace OHOS {
namespace DevStandbyMgr {
class ConditionType {
public:
    enum Type : uint32_t {
        DAY_STANDBY = 1,
        NIGHT_STANDBY = 1 << 1,
    };
};

struct TimeLtdProcess {
    std::string name_;
    int32_t maxDurationLim_;

    bool operator < (const TimeLtdProcess& rhs) const
    {
        return name_ < rhs.name_;
    }
};

struct DefaultResourceConfig {
    bool isAllow_;
    std::vector<uint32_t> conditions_;
    std::vector<std::string> processes_;
    std::vector<std::string> apps_;
    std::vector<TimeLtdProcess> timeLtdProcesses_;
    std::vector<TimeLtdProcess> timeLtdApps_;
};

struct TimerClockApp {
    std::string name_;
    int32_t timerPeriod_;
    bool isTimerClock_;
};

struct TimerResourceConfig {
    bool isAllow_;
    std::vector<uint32_t> conditions_;
    std::vector<TimerClockApp> timerClockApps_;
};

class StandbyConfigManager {
    DECLARE_SINGLE_INSTANCE(StandbyConfigManager);
public:
    ErrCode Init();
    const std::string& GetPluginName();
    bool GetStandbySwitch(const std::string& switchName);
    int32_t GetStandbyParam(const std::string& paramName);
    bool GetStrategySwitch(const std::string& switchName);
    bool GetHalfHourSwitch(const std::string& switchName);
    std::shared_ptr<std::vector<DefaultResourceConfig>> GetResCtrlConfig(const std::string& switchName);
    const std::vector<TimerResourceConfig>& GetTimerResConfig();
    const std::vector<std::string>& GetStrategyConfigList();
    std::vector<int32_t> GetStandbyDurationList(const std::string& switchName);

    std::set<TimeLtdProcess> GetEligibleAllowTimeConfig(const std::string& paramName,
        uint32_t condition, bool isAllow, bool isApp);
    std::set<std::string> GetEligiblePersistAllowConfig(const std::string& paramName,
        uint32_t condition, bool isAllow, bool isApp);
    int32_t GetMaxDuration(const std::string& name, const std::string& paramName, uint32_t condition, bool isApp);

    void DumpSetDebugMode(bool debugMode);
    void DumpSetSwitch(const std::string& switchName, bool switchStatus, std::string& result);
    void DumpSetParameter(const std::string& paramName, int32_t paramValue, std::string& result);

    /**
     * @brief dump config info
     */
    void DumpStandbyConfigInfo(std::string& result);
private:
    template<typename T> std::set<T> GetEligibleAllowConfig(const std::string& paramName,
        uint32_t condition, bool isAllow, bool isApp, const std::function<void(bool, std::set<T>&,
        const DefaultResourceConfig&)>& func);
    template<typename T> T
        GetConfigWithName(const std::string& switchName, std::unordered_map<std::string, T>& configMap);

    std::vector<std::string> GetConfigFileList(const std::string& relativeConfigPath);
    bool ParseDeviceStanbyConfig(const nlohmann::json& devStandbyConfigRoot);
    bool ParseStandbyConfig(const nlohmann::json& standbyConfig);
    bool ParseIntervalList(const nlohmann::json& standbyIntervalList);
    bool ParseStrategyListConfig(const nlohmann::json& standbyListConfig);
    bool ParseHalfHourSwitchConfig(const nlohmann::json& halfHourSwitchConfig);
    bool ParseResCtrlConfig(const nlohmann::json& resCtrlConfigRoot);
    bool ParseTimerResCtrlConfig(const nlohmann::json& resConfigArray);
    bool ParseDefaultResCtrlConfig(const std::string& resCtrlKey, const nlohmann::json& resConfigArray);
    bool ParseCommonResCtrlConfig(const nlohmann::json& sigleConfigItem, DefaultResourceConfig& resCtrlConfig);
    void ParseTimeLimitedConfig(const nlohmann::json& singleConfigItem, const std::string& key,
        std::vector<TimeLtdProcess>& resCtrlConfig);
    uint32_t ParseCondition(const std::string& conditionStr);
    template<typename T> void DumpResCtrlConfig(const char* name, const std::vector<T>& configArray,
        std::stringstream& stream, const std::function<void(const T&)>& func);
private:
    std::mutex configMutex_;
    std::string pluginName_;
    std::unordered_map<std::string, bool> standbySwitchMap_;
    std::unordered_map<std::string, int32_t> standbyParaMap_;
    std::unordered_map<std::string, bool> strategySwitchMap_;
    std::vector<std::string> strategyList_;
    std::unordered_map<std::string, bool> halfhourSwitchMap_;
    std::unordered_map<std::string, std::shared_ptr<std::vector<DefaultResourceConfig>>> defaultResourceConfigMap_;
    std::vector<TimerResourceConfig> timerResConfigList_;
    std::unordered_map<std::string, std::vector<int32_t>> intervalListMap_;

    std::unordered_map<std::string, bool> backStandbySwitchMap_;
    std::unordered_map<std::string, int32_t> backStandbyParaMap_;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_POLICY_INCLUDE_STANDBY_CONFIG_MANAGER_H
