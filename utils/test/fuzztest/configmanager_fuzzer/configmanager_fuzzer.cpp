/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "configmanager_fuzzer.h"
#include "securec.h"
#include "json_utils.h"
#include "nlohmann/json.hpp"
#include "standby_config_manager.h"


namespace OHOS {
namespace DevStandbyMgr {
    constexpr size_t U32_AT_SIZE = 6;
    const std::string TAG_APPS_LIMIT = "apps_limit";
    const std::string TAG_TEST = "test";
    const std::string TAG_TEST_ONE = "test01";
    bool g_initFlag = false;
    const uint8_t *g_baseFuzzData = nullptr;
    size_t g_baseFuzzSize = 0;
    size_t g_baseFuzzPos;

    template <class T> T GetData()
    {
        T object{};
        size_t objectSize = sizeof(object);
        if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
            return object;
        }
        errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
        if (ret != EOK) {
            return {};
        }
        g_baseFuzzPos += objectSize;
        return object;
    }

    void PreciseCoverageParseTimeLimitedConfig()
    {
        auto func = [](bool isApp, std::set<TimeLtdProcess>& eligibleResCtrlConfig,
            const DefaultResourceConfig& config) {
        };
        auto defaultResConfigPtr = std::make_shared<std::vector<DefaultResourceConfig>>();
        DefaultResourceConfig defaultResourceConfig;
        defaultResConfigPtr->emplace_back(std::move(defaultResourceConfig));
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            defaultResourceConfigMap_[TAG_TEST] = defaultResConfigPtr;
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            GetEligibleAllowTimeConfig(TAG_TEST, 0, false, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            GetEligibleAllowConfig<TimeLtdProcess>(TAG_TEST, 0, false, false, func);
        DefaultResourceConfig defaultResourceConfig01;
        std::vector<uint32_t> conditions {0};
        defaultResourceConfig01.conditions_ = conditions;
        defaultResConfigPtr->emplace_back(std::move(defaultResourceConfig01));
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            defaultResourceConfigMap_[TAG_TEST_ONE] = defaultResConfigPtr;
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            GetEligibleAllowTimeConfig(TAG_TEST_ONE, 0, false, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            GetEligibleAllowTimeConfig(TAG_TEST_ONE, 0, false, true);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            GetEligibleAllowConfig<TimeLtdProcess>(TAG_TEST_ONE, 0, false, false, func);
        nlohmann::json jsonValue = nlohmann::json::parse("{\"apps_limit\":[\"1\",\"2\"]}", nullptr, false);
        std::vector<TimeLtdProcess> timeLimitedConfig {};
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            ParseTimeLimitedConfig(jsonValue, TAG_APPS_LIMIT, timeLimitedConfig);
        jsonValue = nlohmann::json::parse("{\"apps_limit\":[{\"name\":\"bundleName\"}]}", nullptr, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            ParseTimeLimitedConfig(jsonValue, TAG_APPS_LIMIT, timeLimitedConfig);
        jsonValue = nlohmann::json::parse("{\"apps_limit\":[{\"name\":\"bundleName\", \"duration\":0}]}",
            nullptr, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->
            ParseTimeLimitedConfig(jsonValue, TAG_APPS_LIMIT, timeLimitedConfig);
    }

    void PreciseCoverageParseTimerResCtrlConfig()
    {
        nlohmann::json resConfigArray = nlohmann::json::parse("{}", nullptr, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseDefaultResCtrlConfig("test", resConfigArray);
        StandbyConfigManager::GetInstance()->ParseStrategyListConfig(resConfigArray);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseResCtrlConfig(resConfigArray);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);
        resConfigArray = nlohmann::json::parse("{\"condition\":[]}", nullptr, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseDefaultResCtrlConfig("test", resConfigArray);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);

        std::string content = "{\"condition\":[\"day\"], \"action\":\"allow\"}";
        resConfigArray = nlohmann::json::parse(content, nullptr, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseDefaultResCtrlConfig("test", resConfigArray);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);

        content = "{\"condition\":[\"day\"], \"action\":\"allow\",\"time_clock_apps\":[]}";
        resConfigArray = nlohmann::json::parse(content, nullptr, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);

        content = "[{\"condition\":[\"day\"], \"action\":\"allow\",\"time_clock_apps\":[{\"name\":\"bundleName\"}]}]";
        resConfigArray = nlohmann::json::parse(content, nullptr, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);

        content = "[{\"condition\":[\"day\"], \"action\":\"allow\",\"time_clock_apps\":[{\"name\":\"bundleName\","\
        "\"timer_clock\":true, \"timer_period\":0}]}]";
        resConfigArray = nlohmann::json::parse(content, nullptr, false);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);
    }

    void PreciseCoverage()
    {
        if (g_initFlag) {
            return;
        }
        g_initFlag = true;
        DelayedSingleton<StandbyConfigManager>::GetInstance()->Init();
        PreciseCoverageParseTimeLimitedConfig();
        PreciseCoverageParseTimerResCtrlConfig();
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetPluginName();
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetStrategyConfigList();
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetTimerResConfig();
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
    {
        g_baseFuzzData = data;
        g_baseFuzzSize = size;
        g_baseFuzzPos = 0;
        uint32_t  condition = GetData<uint32_t>();
        bool debugMode = GetData<bool>();
        bool isAllow = GetData<bool>();
        std::string str((const char *) g_baseFuzzData + g_baseFuzzPos, g_baseFuzzSize - g_baseFuzzPos);
        PreciseCoverage();
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetStandbyDurationList(str);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetStandbySwitch(str);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetStandbyParam(str);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetStrategySwitch(str);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetHalfHourSwitch(str);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetResCtrlConfig(str);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetMaxDuration(str, str,
            condition, debugMode);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetEligibleAllowTimeConfig(str,
            condition, debugMode, isAllow);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->GetEligiblePersistAllowConfig(str,
            condition, debugMode, isAllow);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->DumpSetDebugMode(debugMode);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->DumpSetSwitch(str, debugMode, str);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->DumpSetParameter(str, condition, str);
        DelayedSingleton<StandbyConfigManager>::GetInstance()->DumpStandbyConfigInfo(str);
        return true;
    }
} // namespace DevStandbyMgr
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::DevStandbyMgr::U32_AT_SIZE) {
        return 0;
    }
    OHOS::DevStandbyMgr::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
