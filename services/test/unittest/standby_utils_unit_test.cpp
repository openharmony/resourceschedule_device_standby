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

#include "gtest/gtest.h"
#include "gtest/hwext/gtest-multithread.h"

#include "standby_config_manager.h"
#include "nlohmann/json.hpp"

#include "standby_service_log.h"
#include "json_utils.h"
#include "common_constant.h"

using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    const std::string JSON_KEY = "key";
    const std::string JSON_ERROR_KEY = "error_key";
    const std::string TAG_APPS_LIMIT = "apps_limit";
}
class StandbyUtilsUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

void StandbyUtilsUnitTest::SetUpTestCase()
{
    StandbyConfigManager::GetInstance()->Init();
}

/**
 * @tc.name: StandbyUtilsUnitTest_002
 * @tc.desc: test GetInt32FromJsonValue.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_002, TestSize.Level1)
{
    nlohmann::json jsonValue {};
    int32_t value {0};
    bool ret = JsonUtils::GetInt32FromJsonValue(jsonValue, "", value);
    EXPECT_FALSE(ret);
    jsonValue = nlohmann::json::parse("{\"key\":1}", nullptr, false);
    JsonUtils::GetInt32FromJsonValue(jsonValue, "", value);
    JsonUtils::GetInt32FromJsonValue(jsonValue, JSON_KEY, value);
    JsonUtils::GetInt32FromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    jsonValue = nlohmann::json::parse("{\"key\":\"1\"}", nullptr, false);
    ret = JsonUtils::GetInt32FromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    EXPECT_FALSE(ret);
    JsonUtils::GetInt32FromJsonValue(jsonValue, JSON_KEY, value);
}

/**
 * @tc.name: StandbyUtilsUnitTest_003
 * @tc.desc: test GetBoolFromJsonValue.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_003, TestSize.Level1)
{
    nlohmann::json jsonValue {};
    bool value {false};
    bool ret = JsonUtils::GetBoolFromJsonValue(jsonValue, "", value);
    EXPECT_FALSE(ret);
    jsonValue = nlohmann::json::parse("{\"key\":true}", nullptr, false);
    JsonUtils::GetBoolFromJsonValue(jsonValue, "", value);
    JsonUtils::GetBoolFromJsonValue(jsonValue, JSON_KEY, value);
    JsonUtils::GetBoolFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    jsonValue = nlohmann::json::parse("{\"key\":\"true\"}", nullptr, false);
    ret = JsonUtils::GetBoolFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    EXPECT_FALSE(ret);
    JsonUtils::GetBoolFromJsonValue(jsonValue, JSON_KEY, value);
}

/**
 * @tc.name: StandbyUtilsUnitTest_004
 * @tc.desc: test GetStringFromJsonValue.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_004, TestSize.Level1)
{
    nlohmann::json jsonValue {};
    std::string value {""};
    bool ret = JsonUtils::GetStringFromJsonValue(jsonValue, "", value);
    EXPECT_FALSE(ret);
    jsonValue = nlohmann::json::parse("{\"key\":\"str\"}", nullptr, false);
    JsonUtils::GetStringFromJsonValue(jsonValue, "", value);
    JsonUtils::GetStringFromJsonValue(jsonValue, JSON_KEY, value);
    JsonUtils::GetStringFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    jsonValue = nlohmann::json::parse("{\"key\":1}", nullptr, false);
    ret = JsonUtils::GetStringFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StandbyUtilsUnitTest_005
 * @tc.desc: test GetObjFromJsonValue.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_005, TestSize.Level1)
{
    nlohmann::json jsonValue {};
    nlohmann::json value {};
    bool ret = JsonUtils::GetObjFromJsonValue(jsonValue, "", value);
    EXPECT_FALSE(ret);
    jsonValue = nlohmann::json::parse("{\"key\":{\"value\":1}}", nullptr, false);
    JsonUtils::GetObjFromJsonValue(jsonValue, "", value);
    JsonUtils::GetObjFromJsonValue(jsonValue, JSON_KEY, value);
    JsonUtils::GetObjFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    jsonValue = nlohmann::json::parse("{\"key\":\"str\"}", nullptr, false);
    ret = JsonUtils::GetObjFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StandbyUtilsUnitTest_006
 * @tc.desc: test GetArrayFromJsonValue.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_006, TestSize.Level1)
{
    nlohmann::json jsonValue {};
    nlohmann::json value {};
    bool ret = JsonUtils::GetArrayFromJsonValue(jsonValue, "", value);
    EXPECT_FALSE(ret);
    jsonValue = nlohmann::json::parse("{\"key\":[1,2.3]}", nullptr, false);
    JsonUtils::GetArrayFromJsonValue(jsonValue, "", value);
    JsonUtils::GetArrayFromJsonValue(jsonValue, JSON_KEY, value);
    JsonUtils::GetArrayFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    jsonValue = nlohmann::json::parse("{\"key\":true}", nullptr, false);
    ret = JsonUtils::GetArrayFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StandbyUtilsUnitTest_007
 * @tc.desc: test GetStrArrFromJsonValue.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_007, TestSize.Level1)
{
    nlohmann::json jsonValue {};
    std::vector<std::string> value {};
    bool ret = JsonUtils::GetStrArrFromJsonValue(jsonValue, "", value);
    EXPECT_FALSE(ret);
    jsonValue = nlohmann::json::parse("{\"key\":[\"1\",\"2\"]}", nullptr, false);
    JsonUtils::GetStrArrFromJsonValue(jsonValue, "", value);
    JsonUtils::GetStrArrFromJsonValue(jsonValue, JSON_KEY, value);
    JsonUtils::GetStrArrFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    jsonValue = nlohmann::json::parse("{\"key\":true}", nullptr, false);
    ret = JsonUtils::GetStrArrFromJsonValue(jsonValue, JSON_ERROR_KEY, value);
    EXPECT_FALSE(ret);
    jsonValue = nlohmann::json::parse("{\"key\":[1,2]}", nullptr, false);
    JsonUtils::GetStrArrFromJsonValue(jsonValue, JSON_KEY, value);
}

/**
 * @tc.name: StandbyUtilsUnitTest_008
 * @tc.desc: test GetRealPath.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_008, TestSize.Level1)
{
    std::string partialPath (PATH_MAX + 1, 'a');
    std::string fullPath;
    EXPECT_FALSE(JsonUtils::GetRealPath(partialPath, fullPath));
    JsonUtils::CreateNodeFile("/data/service/el1/public/device_standby/allow_record");
    JsonUtils::CreateNodeFile("/data/service/el1/public/device_standby/allow_record");
    nlohmann::json jsonValue;
    JsonUtils::DumpJsonValueToFile(jsonValue, "/data/service/el1/public/device_standby/record");
}

/**
 * @tc.name: StandbyUtilsUnitTest_009
 * @tc.desc: test ParseCondition.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_009, TestSize.Level1)
{
    EXPECT_TRUE(StandbyConfigManager::GetInstance()->ParseCondition("test") == 0);
}

/**
 * @tc.name: StandbyUtilsUnitTest_010
 * @tc.desc: test DUMP.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_010, TestSize.Level1)
{
    std::string result {""};
    StandbyConfigManager::GetInstance()->DumpSetDebugMode(true);
    StandbyConfigManager::GetInstance()->DumpSetDebugMode(false);
    StandbyConfigManager::GetInstance()->DumpSetSwitch("test", true, result);
    StandbyConfigManager::GetInstance()->DumpSetSwitch(NAP_SWITCH, true, result);
    StandbyConfigManager::GetInstance()->DumpSetParameter("test", 0, result);
    StandbyConfigManager::GetInstance()->DumpSetParameter(NAP_TIMEOUT, 0, result);

    StandbyConfigManager::GetInstance()->DumpStandbyConfigInfo(result);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: StandbyUtilsUnitTest_011
 * @tc.desc: test ParseTimeLimitedConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_011, TestSize.Level1)
{
    nlohmann::json jsonValue = nlohmann::json::parse("{\"apps_limit\":[\"1\",\"2\"]}", nullptr, false);
    std::vector<TimeLtdProcess> timeLimitedConfig {};
    StandbyConfigManager::GetInstance()->ParseTimeLimitedConfig(jsonValue, TAG_APPS_LIMIT, timeLimitedConfig);
    EXPECT_TRUE(timeLimitedConfig.empty());
    jsonValue = nlohmann::json::parse("{\"apps_limit\":[{\"name\":\"bundleName\"}]}", nullptr, false);
    StandbyConfigManager::GetInstance()->ParseTimeLimitedConfig(jsonValue, TAG_APPS_LIMIT, timeLimitedConfig);
    jsonValue = nlohmann::json::parse("{\"apps_limit\":[{\"name\":\"bundleName\", \"duration\":0}]}", nullptr, false);
    StandbyConfigManager::GetInstance()->ParseTimeLimitedConfig(jsonValue, TAG_APPS_LIMIT, timeLimitedConfig);
}

/**
 * @tc.name: StandbyUtilsUnitTest_012
 * @tc.desc: test ParseCommonResCtrlConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_012, TestSize.Level1)
{
    nlohmann::json jsonValue = nlohmann::json::parse("{}", nullptr, false);
    DefaultResourceConfig resCtrlConfig {};
    StandbyConfigManager::GetInstance()->ParseCommonResCtrlConfig(jsonValue, resCtrlConfig);
    EXPECT_TRUE(resCtrlConfig.conditions_.empty());
    jsonValue = nlohmann::json::parse("{\"action\":\"allow\"}", nullptr, false);
    StandbyConfigManager::GetInstance()->ParseCommonResCtrlConfig(jsonValue, resCtrlConfig);

    jsonValue = nlohmann::json::parse("{\"condition\":[\"day_standby\"], \"action\":\"allow\"}",
        nullptr, false);
    StandbyConfigManager::GetInstance()->ParseCommonResCtrlConfig(jsonValue, resCtrlConfig);
    jsonValue = nlohmann::json::parse("{\"condition\":0, \"action\":0}",
        nullptr, false);
    StandbyConfigManager::GetInstance()->ParseCommonResCtrlConfig(jsonValue, resCtrlConfig);
    jsonValue = nlohmann::json::parse("{\"condition\":[\"day\"], \"action\":\"allow\"}", nullptr, false);
    StandbyConfigManager::GetInstance()->ParseCommonResCtrlConfig(jsonValue, resCtrlConfig);
}

/**
 * @tc.name: StandbyUtilsUnitTest_013
 * @tc.desc: test ParseDefaultResCtrlConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_013, TestSize.Level1)
{
    nlohmann::json resConfigArray = nlohmann::json::parse("{}", nullptr, false);
    EXPECT_FALSE(StandbyConfigManager::GetInstance()->ParseDefaultResCtrlConfig("test", resConfigArray));
    StandbyConfigManager::GetInstance()->ParseStrategyListConfig(resConfigArray);
    StandbyConfigManager::GetInstance()->ParseResCtrlConfig(resConfigArray);
    StandbyConfigManager::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);
    resConfigArray = nlohmann::json::parse("{\"condition\":[]}", nullptr, false);
    StandbyConfigManager::GetInstance()->ParseDefaultResCtrlConfig("test", resConfigArray);
    StandbyConfigManager::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);

    std::string content = "{\"condition\":[\"day\"], \"action\":\"allow\"}";
    resConfigArray = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseDefaultResCtrlConfig("test", resConfigArray);
    StandbyConfigManager::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);

    content = "{\"condition\":[\"day\"], \"action\":\"allow\",\"time_clock_apps\":[]}";
    resConfigArray = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);

    content = "{\"condition\":[\"day\"], \"action\":\"allow\",\"time_clock_apps\":[{\"name\":\"bundleName\"}]}";
    resConfigArray = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);

    content = "{\"condition\":[\"day\"], \"action\":\"allow\",\"time_clock_apps\":[{\"name\":\"bundleName\","\
        "\"timer_clock\":true, \"timer_period\":0}]}";
    resConfigArray = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseTimerResCtrlConfig(resConfigArray);
}

/**
 * @tc.name: StandbyUtilsUnitTest_014
 * @tc.desc: test ParseDeviceStanbyConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StandbyUtilsUnitTest, StandbyUtilsUnitTest_014, TestSize.Level1)
{
    nlohmann::json devStandbyConfigRoot = nlohmann::json::parse("{}", nullptr, false);
    EXPECT_TRUE(StandbyConfigManager::GetInstance()->ParseDeviceStanbyConfig(devStandbyConfigRoot));
    std::string content = "{\"halfhour_switch_setting\":[\"test\":0]}";
    devStandbyConfigRoot = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseDeviceStanbyConfig(devStandbyConfigRoot);
    StandbyConfigManager::GetInstance()->ParseHalfHourSwitchConfig(devStandbyConfigRoot);
    content = "{\"halfhour_switch_setting\":[\"test\":true]}";
    devStandbyConfigRoot = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseHalfHourSwitchConfig(devStandbyConfigRoot);
    content = "{\"standby\":[\"test\":0]}";
    devStandbyConfigRoot = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseDeviceStanbyConfig(devStandbyConfigRoot);
    content = "{\"detect_list\":[\"test\":0]}";
    devStandbyConfigRoot = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseDeviceStanbyConfig(devStandbyConfigRoot);
    content = "{\"maintenance_list\":[\"test\":[1, 2, 3]]}";
    devStandbyConfigRoot = nlohmann::json::parse(content, nullptr, false);
    StandbyConfigManager::GetInstance()->ParseDeviceStanbyConfig(devStandbyConfigRoot);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS