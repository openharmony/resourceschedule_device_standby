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
#include "gtest/gtest.h"
#include "gtest/hwext/gtest-multithread.h"
#include "allow_record.h"

using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    constexpr int32_t DEFAULT_UID = 0;
    constexpr int32_t DEFAULT_PID = 0;
    const std::string DEFAULT_BUNDLE_NAME = "test";
    const uint32_t DEFAULT_ALLOW_TYPE = 1;
    const uint32_t DEFAULT_REASON_CODE = 1;
    const uint32_t DEFAULT_ALLOW_TYPE_INDEX = 1;
    const int64_t DEFAULT_END_TIME = 1;
    const std::string DEFAULT_REASON = "test";
}
class AllowRecordUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: AllowRecordUnitTest_001
 * @tc.desc: test AllowRecord ParseFromJson
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AllowRecordUnitTest, AllowRecordUnitTest_001, TestSize.Level1)
{
    std::shared_ptr<AllowRecord> allowRecord = std::make_shared<AllowRecord>();
    nlohmann::json payload;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), false);
    payload["uid"] = DEFAULT_UID;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), false);
    payload["pid"] = DEFAULT_PID;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), false);
    payload["name"] = DEFAULT_BUNDLE_NAME;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), false);
    payload["allowType"] = DEFAULT_ALLOW_TYPE;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), false);
    payload["reasonCode"] = DEFAULT_REASON_CODE;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), true);
    payload["allowTimeList"] = DEFAULT_UID;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), true);
    nlohmann::json persistTime;
    std::vector<nlohmann::json> vector{persistTime};
    payload["allowTimeList"] = vector;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), false);
    persistTime["allowTypeIndex"] = DEFAULT_ALLOW_TYPE_INDEX;
    std::vector<nlohmann::json> vector1{persistTime};
    payload["allowTimeList"] = vector1;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), false);
    persistTime["endTime"] = DEFAULT_END_TIME;
    std::vector<nlohmann::json> vector2{persistTime};
    payload["allowTimeList"] = vector2;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), false);
    persistTime["reason"] = DEFAULT_REASON;
    std::vector<nlohmann::json> vector3{persistTime};
    payload["allowTimeList"] = vector3;
    EXPECT_EQ(allowRecord->ParseFromJson(payload), true);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
