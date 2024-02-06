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

#include "standby_config_manager.h"

#include <functional>
#include <string>
#include <sstream>
#include <unistd.h>

#ifdef STANDBY_CONFIG_POLICY_ENABLE
#include "config_policy_utils.h"
#endif
#include "json_utils.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    const std::string DEFAULT_CONFIG_ROOT_DIR = "/system";
    const std::string STANDBY_CONFIG_PATH = "/etc/standby_service/device_standby_config.json";
    const std::string STRATEGY_CONFIG_PATH = "/etc/standby_service/standby_strategy_config.json";
    const std::string TAG_PLUGIN_NAME = "plugin_name";
    const std::string TAG_STANDBY = "standby";
    const std::string TAG_MAINTENANCE_LIST = "maintenance_list";
    const std::string TAG_DETECT_LIST = "detect_list";
    const std::string TAG_STRATEGY_LIST = "strategy_list";
    const std::string TAG_HALFHOUR_SWITCH_SETTING = "halfhour_switch_setting";

    const std::string TAG_CONDITION = "condition";
    const std::string TAG_ACTION = "action";
    const std::string TAG_ALLOW = "allow";
    const std::string TAG_PROCESSES = "processes";
    const std::string TAG_APPS = "apps";
    const std::string TAG_PROCESSES_LIMIT = "processes_limit";
    const std::string TAG_TIME_CLOCK_APPS = "time_clock_apps";
    const std::string TAG_APPS_LIMIT = "apps_limit";
    const std::string TAG_NAME = "name";
    const std::string TAG_MAX_DURATION_LIM = "duration";

    const std::string TAG_TIMER = "TIMER";
    const std::string TAG_TIMER_CLOCK = "timer_clock";
    const std::string TAG_TIMER_PERIOD = "timer_period";

    const char TAG_CONDITION_DELIM = '&';
    const std::string TAG_DAY_STANDBY = "day_standby";
    const std::string TAG_NIGHT_STANDBY = "night_standby";
    const std::string TAG_SCREENOFF = "screenoff";
    const std::string TAG_SCREENOFF_HALFHOUR = "screenoff_halfhour";
    const std::unordered_map<std::string, ConditionType::Type> conditionMap = {
        {TAG_DAY_STANDBY, ConditionType::DAY_STANDBY},
        {TAG_NIGHT_STANDBY, ConditionType::NIGHT_STANDBY},
    };
}

IMPLEMENT_SINGLE_INSTANCE(StandbyConfigManager);

StandbyConfigManager::StandbyConfigManager() {}

StandbyConfigManager::~StandbyConfigManager() {}

ErrCode StandbyConfigManager::Init()
{
    STANDBYSERVICE_LOGI("start to read config");

    std::vector<std::string> configFileList = GetConfigFileList(STANDBY_CONFIG_PATH);
    for (const auto& configFile : configFileList) {
        nlohmann::json devStandbyConfigRoot;
        // if failed to load one json file, read next config file
        if (!JsonUtils::LoadJsonValueFromFile(devStandbyConfigRoot, configFile)) {
            STANDBYSERVICE_LOGE("load config file %{public}s failed", configFile.c_str());
            continue;
        }
        if (!ParseDeviceStanbyConfig(devStandbyConfigRoot)) {
            STANDBYSERVICE_LOGE("parse config file %{public}s failed", configFile.c_str());
        }
    }

    configFileList = GetConfigFileList(STRATEGY_CONFIG_PATH);
    for (const auto& configFile : configFileList) {
        nlohmann::json resCtrlConfigRoot;
        if (!JsonUtils::LoadJsonValueFromFile(resCtrlConfigRoot, configFile)) {
            STANDBYSERVICE_LOGE("load config file %{public}s failed", configFile.c_str());
            continue;
        }
        if (!ParseResCtrlConfig(resCtrlConfigRoot)) {
            STANDBYSERVICE_LOGE("parse config file %{public}s failed", configFile.c_str());
        }
    }
    return ERR_OK;
}

std::vector<std::string> StandbyConfigManager::GetConfigFileList(const std::string& relativeConfigPath)
{
    std::list<std::string> rootDirList;
#ifdef STANDBY_CONFIG_POLICY_ENABLE
        auto cfgDirList = GetCfgDirList();
        if (cfgDirList != nullptr) {
            for (const auto &cfgDir : cfgDirList->paths) {
                if (cfgDir == nullptr) {
                    continue;
                }
                STANDBYSERVICE_LOGD("cfgDir: %{public}s ", cfgDir);
                rootDirList.emplace_back(cfgDir);
            }
            FreeCfgDirList(cfgDirList);
        }
#endif
    if (std::find(rootDirList.begin(), rootDirList.end(), DEFAULT_CONFIG_ROOT_DIR)
        == rootDirList.end()) {
        rootDirList.emplace_front(DEFAULT_CONFIG_ROOT_DIR);
    }
    std::string baseRealPath;
    std::vector<std::string> configFilesList;
    for (auto configDir : rootDirList) {
        if (JsonUtils::GetRealPath(configDir + relativeConfigPath, baseRealPath)
            && access(baseRealPath.c_str(), F_OK) == ERR_OK) {
            STANDBYSERVICE_LOGD("Get valid base config file: %{public}s", baseRealPath.c_str());
            configFilesList.emplace_back(baseRealPath);
        }
    }
    return configFilesList;
}

const std::string& StandbyConfigManager::GetPluginName()
{
    return pluginName_;
}

bool StandbyConfigManager::GetStandbySwitch(const std::string& switchName)
{
    return GetConfigWithName(switchName, standbySwitchMap_);
}

int32_t StandbyConfigManager::GetStandbyParam(const std::string& paramName)
{
    return GetConfigWithName(paramName, standbyParaMap_);
}

bool StandbyConfigManager::GetStrategySwitch(const std::string& switchName)
{
    return GetConfigWithName(switchName, strategySwitchMap_);
}

bool StandbyConfigManager::GetHalfHourSwitch(const std::string& switchName)
{
    return GetConfigWithName(switchName, halfhourSwitchMap_);
}

std::shared_ptr<std::vector<DefaultResourceConfig>> StandbyConfigManager::GetResCtrlConfig(const
    std::string& switchName)
{
    return GetConfigWithName(switchName, defaultResourceConfigMap_);
}

template<typename T>
T StandbyConfigManager::GetConfigWithName(const std::string& switchName,
    std::unordered_map<std::string, T>& configMap)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    auto iter = configMap.find(switchName);
    if (iter == configMap.end()) {
        STANDBYSERVICE_LOGW("failed to find config %{public}s", switchName.c_str());
        return T{};
    }
    return iter->second;
}

const std::vector<TimerResourceConfig>& StandbyConfigManager::GetTimerResConfig()
{
    return timerResConfigList_;
}

const std::vector<std::string>& StandbyConfigManager::GetStrategyConfigList()
{
    return strategyList_;
}

std::vector<int32_t> StandbyConfigManager::GetStandbyDurationList(const std::string& switchName)
{
    return GetConfigWithName(switchName, intervalListMap_);
}

int32_t StandbyConfigManager::GetMaxDuration(const std::string& name, const std::string& paramName,
    uint32_t condition, bool isApp)
{
    auto eligibleAllowTimeList = GetEligibleAllowTimeConfig(paramName, condition, true, isApp);
    auto findConfigTask = [&name](const auto& it) { return it.name_ == name; };
    auto it = std::find_if(eligibleAllowTimeList.begin(), eligibleAllowTimeList.end(), findConfigTask);
    if (it == eligibleAllowTimeList.end()) {
        return 0;
    } else {
        return it->maxDurationLim_;
    }
}

template<typename T> std::set<T> StandbyConfigManager::GetEligibleAllowConfig(const std::string& paramName,
    uint32_t condition, bool isAllow, bool isApp, const std::function<void(bool, std::set<T>&,
    const DefaultResourceConfig&)>& func)
{
    if (defaultResourceConfigMap_.find(paramName) == defaultResourceConfigMap_.end()) {
        return {};
    }
    std::set<T> eligibleResCtrlConfig;
    const auto& resCtrlConfig = *(defaultResourceConfigMap_.find(paramName)->second);
    STANDBYSERVICE_LOGD("find duration from %{public}s, size is %{public}d",
        paramName.c_str(), static_cast<int32_t>(resCtrlConfig.size()));
    for (const auto& config : resCtrlConfig) {
        if (config.isAllow_ != isAllow) {
            continue;
        }
        bool isEligiable {false};
        for (const auto configCondition : config.conditions_) {
            if ((condition & configCondition) == configCondition) {
                isEligiable = true;
                break;
            }
        }
        if (!isEligiable) {
            continue;
        }
        func(isApp, eligibleResCtrlConfig, config);
    }
    STANDBYSERVICE_LOGD("eligibleResCtrlConfig size is %{public}d",
        static_cast<int32_t>(eligibleResCtrlConfig.size()));
    return eligibleResCtrlConfig;
}

std::set<TimeLtdProcess> StandbyConfigManager::GetEligibleAllowTimeConfig(const std::string& paramName,
    uint32_t condition, bool isAllow, bool isApp)
{
    auto func = [](bool isApp, std::set<TimeLtdProcess>& eligibleResCtrlConfig,
        const DefaultResourceConfig& config) {
        if (isApp) {
            eligibleResCtrlConfig.insert(config.timeLtdApps_.begin(), config.timeLtdApps_.end());
        } else {
            eligibleResCtrlConfig.insert(config.timeLtdProcesses_.begin(), config.timeLtdProcesses_.end());
        }
        STANDBYSERVICE_LOGD("after calculate, eligible size is %{public}d",
            static_cast<int32_t>(eligibleResCtrlConfig.size()));
    };
    return GetEligibleAllowConfig<TimeLtdProcess>(paramName, condition, isAllow, isApp, func);
}

std::set<std::string> StandbyConfigManager::GetEligiblePersistAllowConfig(const std::string& paramName,
    uint32_t condition, bool isAllow, bool isApp)
{
    auto func = [](bool isApp, std::set<std::string>& eligibleResCtrlConfig,
        const DefaultResourceConfig& config) {
        if (isApp) {
            eligibleResCtrlConfig.insert(config.apps_.begin(), config.apps_.end());
        } else {
            eligibleResCtrlConfig.insert(config.processes_.begin(), config.processes_.end());
        }
    };
    return GetEligibleAllowConfig<std::string>(paramName, condition, isAllow, isApp, func);
}

bool StandbyConfigManager::ParseDeviceStanbyConfig(const nlohmann::json& devStandbyConfigRoot)
{
    nlohmann::json standbyConfig;
    nlohmann::json detectlist;
    nlohmann::json standbySwitchConfig;
    nlohmann::json standbyListConfig;
    nlohmann::json standbyIntervalList;

    JsonUtils::GetStringFromJsonValue(devStandbyConfigRoot, TAG_PLUGIN_NAME, pluginName_);
    if (JsonUtils::GetObjFromJsonValue(devStandbyConfigRoot, TAG_STANDBY, standbyConfig) &&
        !ParseStandbyConfig(standbyConfig)) {
        STANDBYSERVICE_LOGW("failed to parse standby config in %{public}s", STANDBY_CONFIG_PATH.c_str());
        return false;
    }
    if (JsonUtils::GetObjFromJsonValue(devStandbyConfigRoot, TAG_DETECT_LIST, detectlist) &&
        !ParseStandbyConfig(detectlist)) {
        STANDBYSERVICE_LOGW("failed to parse detect list in %{public}s", STANDBY_CONFIG_PATH.c_str());
        return false;
    }
    if (JsonUtils::GetObjFromJsonValue(devStandbyConfigRoot, TAG_MAINTENANCE_LIST, standbyIntervalList) &&
        !ParseIntervalList(standbyIntervalList)) {
        STANDBYSERVICE_LOGW("failed to parse standby interval list in %{public}s", STANDBY_CONFIG_PATH.c_str());
        return false;
    }
    if (JsonUtils::GetArrayFromJsonValue(devStandbyConfigRoot, TAG_STRATEGY_LIST, standbyListConfig) &&
        !ParseStrategyListConfig(standbyListConfig)) {
        STANDBYSERVICE_LOGW("failed to parse strategy list config in %{public}s", STANDBY_CONFIG_PATH.c_str());
        return false;
    }

    if (JsonUtils::GetObjFromJsonValue(devStandbyConfigRoot, TAG_HALFHOUR_SWITCH_SETTING, standbyConfig)) {
        if (!ParseHalfHourSwitchConfig(standbyConfig)) {
            STANDBYSERVICE_LOGW("failed to parse halfhour config");
            return false;
        }
    }
    return true;
}

bool StandbyConfigManager::ParseStandbyConfig(const nlohmann::json& standbyConfig)
{
    bool ret = true;
    for (const auto& element : standbyConfig.items()) {
        if (!element.value().is_primitive()) {
            STANDBYSERVICE_LOGW("there is unexpected type of key in standby config %{public}s", element.key().c_str());
            ret = false;
            continue;
        }
        if (element.value().is_boolean()) {
            standbySwitchMap_[element.key()] = element.value().get<bool>();
        } else if (element.value().is_number_integer()) {
            if (element.value().get<int32_t>() < 0) {
                STANDBYSERVICE_LOGW("there is negative value in standby config %{public}s", element.key().c_str());
                ret = false;
                continue;
            }
            standbyParaMap_[element.key()] = element.value().get<int32_t>();
        }
    }
    return ret;
}

bool StandbyConfigManager::ParseIntervalList(const nlohmann::json& standbyIntervalList)
{
    bool ret = true;
    for (const auto& element : standbyIntervalList.items()) {
        if (!element.value().is_array()) {
            STANDBYSERVICE_LOGW("there is unexpected value of %{public}s in standby interval list",
                element.key().c_str());
            ret = false;
            continue;
        }
        std::vector<int32_t> intervalList;
        for (const int32_t interval : element.value()) {
            intervalList.emplace_back(interval);
        }
        intervalListMap_.emplace(element.key(), std::move(intervalList));
    }
    return ret;
}

bool StandbyConfigManager::ParseStrategyListConfig(const nlohmann::json& standbyListConfig)
{
    if (!standbyListConfig.is_array()) {
        STANDBYSERVICE_LOGW("there is error in strategy list config");
        return false;
    }
    strategyList_.clear();
    for (const auto& element : standbyListConfig) {
        strategyList_.emplace_back(element.get<std::string>());
    }
    return true;
}

bool StandbyConfigManager::ParseHalfHourSwitchConfig(const nlohmann::json& halfHourSwitchConfig)
{
    bool ret = true;
    for (const auto& element : halfHourSwitchConfig.items()) {
        if (!element.value().is_boolean()) {
            STANDBYSERVICE_LOGW("there is unexpected type of value in half hour standby switch config %{public}s",
                element.key().c_str());
            ret = false;
            return ret;
        }
        halfhourSwitchMap_[element.key()] = element.value().get<bool>();
    }
    return ret;
}

bool StandbyConfigManager::ParseResCtrlConfig(const nlohmann::json& resCtrlConfigRoot)
{
    bool ret = true;
    for (const auto& element : resCtrlConfigRoot.items()) {
        if (!element.value().is_array()) {
            STANDBYSERVICE_LOGW("there is unexpected type of value in resource control config %{public}s",
                element.key().c_str());
            ret = false;
            continue;
        }
        std::string resCtrlKey = element.key();
        if (!ParseDefaultResCtrlConfig(resCtrlKey, element.value())) {
            STANDBYSERVICE_LOGW("there is error in config of %{public}s", resCtrlKey.c_str());
            ret = false;
            continue;
        }
        // parse exemption config of timer resource
        if (resCtrlKey == TAG_TIMER && !ParseTimerResCtrlConfig(element.value())) {
            STANDBYSERVICE_LOGW("there is error in config of %{public}s", resCtrlKey.c_str());
            ret = false;
            continue;
        }
    }
    return ret;
}

bool StandbyConfigManager::ParseTimerResCtrlConfig(const nlohmann::json& resConfigArray)
{
    if (!resConfigArray.is_array()) {
        STANDBYSERVICE_LOGW("the value of timer config should be an array");
        return false;
    }
    timerResConfigList_.clear();
    for (const auto &singleConfigItem : resConfigArray) {
        TimerResourceConfig timerResourceConfig;
        if (!singleConfigItem.contains(TAG_TIME_CLOCK_APPS)) {
            timerResConfigList_.emplace_back(std::move(timerResourceConfig));
            continue;
        }
        const nlohmann::json& limitedAppItems = singleConfigItem.at(TAG_TIME_CLOCK_APPS);
        for (const auto &singleLtdAppItem : limitedAppItems) {
            TimerClockApp timerClockApp;
            if (!JsonUtils::GetStringFromJsonValue(singleLtdAppItem, TAG_NAME, timerClockApp.name_) ||
                (!JsonUtils::GetBoolFromJsonValue(singleLtdAppItem, TAG_TIMER_CLOCK, timerClockApp.isTimerClock_) &&
                !JsonUtils::GetInt32FromJsonValue(singleLtdAppItem, TAG_TIMER_PERIOD, timerClockApp.timerPeriod_))) {
                STANDBYSERVICE_LOGW("there is error in timer clock config");
                return false;
            }
            timerResourceConfig.timerClockApps_.emplace_back(std::move(timerClockApp));
        }
        timerResConfigList_.emplace_back(std::move(timerResourceConfig));
    }
    return true;
}

bool StandbyConfigManager::ParseDefaultResCtrlConfig(const std::string& resCtrlKey,
    const nlohmann::json& resConfigArray)
{
    if (!resConfigArray.is_array()) {
        STANDBYSERVICE_LOGW("the value of %{public}s should be an array", resCtrlKey.c_str());
        return false;
    }
    auto defaultResConfigPtr = std::make_shared<std::vector<DefaultResourceConfig>>();
    for (const auto &singleConfigItem : resConfigArray) {
        DefaultResourceConfig defaultResourceConfig;
        if (!ParseCommonResCtrlConfig(singleConfigItem, defaultResourceConfig)) {
            STANDBYSERVICE_LOGW("the value of %{public}s can not be parsed", resCtrlKey.c_str());
            return false;
        }
        defaultResConfigPtr->emplace_back(std::move(defaultResourceConfig));
    }
    defaultResourceConfigMap_[resCtrlKey] = defaultResConfigPtr;
    STANDBYSERVICE_LOGI("succeed to parse the config of %{public}s", resCtrlKey.c_str());
    return true;
}

bool StandbyConfigManager::ParseCommonResCtrlConfig(const nlohmann::json& singleConfigItem,
    DefaultResourceConfig& resCtrlConfig)
{
    if (!singleConfigItem.contains(TAG_ACTION) || !singleConfigItem.contains(TAG_CONDITION)) {
        STANDBYSERVICE_LOGW("there is no necessary field %{public}s or %{public}s",
            TAG_ACTION.c_str(), TAG_CONDITION.c_str());
        return false;
    }
    std::string resCtrlAction;
    std::vector<std::string> conditionItemArray {};
    if (!JsonUtils::GetStringFromJsonValue(singleConfigItem, TAG_ACTION, resCtrlAction) ||
        !JsonUtils::GetStrArrFromJsonValue(singleConfigItem, TAG_CONDITION, conditionItemArray)) {
        STANDBYSERVICE_LOGW("get necessary field %{public}s or %{public}s config failed",
            TAG_ACTION.c_str(), TAG_CONDITION.c_str());
        return false;
    }
    resCtrlConfig.isAllow_ = resCtrlAction == TAG_ALLOW;

    for (const auto &singleConditionItem : conditionItemArray) {
        uint32_t conditionValue = ParseCondition(singleConditionItem);
        if (conditionValue > 0) {
            resCtrlConfig.conditions_.emplace_back(conditionValue);
        }
    }

    JsonUtils::GetStrArrFromJsonValue(singleConfigItem, TAG_PROCESSES, resCtrlConfig.processes_);
    JsonUtils::GetStrArrFromJsonValue(singleConfigItem, TAG_APPS, resCtrlConfig.apps_);
    ParseTimeLimitedConfig(singleConfigItem, TAG_PROCESSES_LIMIT, resCtrlConfig.timeLtdProcesses_);
    ParseTimeLimitedConfig(singleConfigItem, TAG_APPS_LIMIT, resCtrlConfig.timeLtdApps_);
    return true;
}

void StandbyConfigManager::ParseTimeLimitedConfig(const nlohmann::json& singleConfigItem,
    const std::string& key, std::vector<TimeLtdProcess>& timeLimitedConfig)
{
    nlohmann::json timeLimitedItems;
    if (!JsonUtils::GetArrayFromJsonValue(singleConfigItem, key, timeLimitedItems)) {
        return;
    }
    for (const auto &singleLtdItem : timeLimitedItems) {
        std::string name {};
        int32_t duration {0};
        if (!JsonUtils::GetStringFromJsonValue(singleLtdItem, TAG_NAME, name) ||
            !JsonUtils::GetInt32FromJsonValue(singleLtdItem, TAG_MAX_DURATION_LIM, duration)) {
            STANDBYSERVICE_LOGW("there is error in %{public}s config", key.c_str());
            continue;
        }
        timeLimitedConfig.emplace_back(TimeLtdProcess{name, duration});
    }
}

uint32_t StandbyConfigManager::ParseCondition(const std::string& conditionStr)
{
    uint32_t conditionValue = 0;
    std::stringstream ss(conditionStr);
    std::string conditionSubstr;
    while (std::getline(ss, conditionSubstr, TAG_CONDITION_DELIM)) {
        auto iter = conditionMap.find(conditionSubstr);
        if (iter == conditionMap.end()) {
            continue;
        }
        conditionValue |= iter->second;
    }
    return conditionValue;
}

void StandbyConfigManager::DumpSetDebugMode(bool debugMode)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    if (debugMode) {
        backStandbySwitchMap_ = standbySwitchMap_;
        backStandbyParaMap_ = standbyParaMap_;
    } else {
        standbySwitchMap_ = backStandbySwitchMap_;
        standbyParaMap_ = backStandbyParaMap_;
        backStandbySwitchMap_.clear();
        backStandbyParaMap_.clear();
    }
}

void StandbyConfigManager::DumpSetSwitch(const std::string& switchName, bool switchStatus, std::string& result)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    auto iter = standbySwitchMap_.find(switchName);
    if (iter == standbySwitchMap_.end()) {
        result += switchName + " not exist\n";
        return;
    }
    iter->second = switchStatus;
}

void StandbyConfigManager::DumpSetParameter(const std::string& paramName, int32_t paramValue, std::string& result)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    auto iter = standbyParaMap_.find(paramName);
    if (iter == standbyParaMap_.end()) {
        result += paramName + " not exist\n";
        return;
    }
    iter->second = paramValue;
}

void StandbyConfigManager::DumpStandbyConfigInfo(std::string& result)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    std::stringstream stream;
    for (const auto& [switchName, switchVal] : standbySwitchMap_) {
        stream << switchName << ": " << (switchVal ? "true" : "false") << "\n";
    }
    for (const auto& [paraName, paraVal] : standbyParaMap_) {
        stream << paraName << ": " << paraVal << "\n";
    }
    for (const auto& [strategyName, strategyVal] : strategySwitchMap_) {
        stream << strategyName << ": " << (strategyVal ? "true" : "false") << "\n";
    }
    stream << "strategy:";
    for (const auto& strategy : strategyList_) {
        stream << " " << strategy;
    }
    stream << "\n";
    auto printConditions = [&stream](const int32_t& condition) { stream << "\t\t" << condition << " "; };
    auto printProceses = [&stream](const std::string& process) { stream << "\t\t" << process << "\n"; };
    auto printLtdProceses = [&stream](const TimeLtdProcess& timeLtdProcess) {
        stream << "\t\t" << timeLtdProcess.name_ << " " << timeLtdProcess.maxDurationLim_ << "\n";
        };
    for (const auto& [resCtrlKey, resConfigVec] : defaultResourceConfigMap_) {
        for (const auto& resConfig : *resConfigVec) {
            stream << resCtrlKey << ": \n";
            stream << "\tisAllow: " << resConfig.isAllow_ << "\n";
            DumpResCtrlConfig<uint32_t>("conditions", resConfig.conditions_, stream, printConditions);
            stream << "\n";
            DumpResCtrlConfig<std::string>("processes", resConfig.processes_, stream, printProceses);
            DumpResCtrlConfig<std::string>("apps", resConfig.apps_, stream, printProceses);
            DumpResCtrlConfig<TimeLtdProcess>("timeLtdProcesses", resConfig.timeLtdProcesses_,
                stream, printLtdProceses);
            DumpResCtrlConfig<TimeLtdProcess>("timeLtdApps", resConfig.timeLtdApps_, stream, printLtdProceses);
        }
    }
    result += stream.str();
    stream.str("");
    stream.clear();
}

template<typename T> void StandbyConfigManager::DumpResCtrlConfig(const char* name, const std::vector<T>& configArray,
    std::stringstream& stream, const std::function<void(const T&)>& func)
{
    if (configArray.empty()) {
        return;
    }
    stream << "\t" << name << ":\n";
    for_each(configArray.begin(), configArray.end(), func);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS