/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "common_event_manager.h"
#include "mock_common_event.h"

namespace {
    bool g_mockPublishCommonEvent = true;
    bool g_mockSubscribeCommonEvent = true;
    int32_t CLOUD_CONFIG_INDEX = 7;
}

namespace OHOS {
namespace EventFwk {
bool CommonEventManager::PublishCommonEvent(const CommonEventData& data)
{
    return g_mockPublishCommonEvent;
}

bool CommonEventManager::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber>& subscriber)
{
    return g_mockSubscribeCommonEvent;
}

bool CommonEventManager::UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber>& subscriber)
{
    return g_mockSubscribeCommonEvent;
}
}  // namespace EventFwk
namespace DevStandbyMgr {
int g_mockFunctionCallCount = 0;

void MockCommonEvent::MockPublishCommonEvent(bool mockRet)
{
    g_mockPublishCommonEvent = mockRet;
}

void MockCommonEvent::MockSubscribeCommonEvent(bool mockRet)
{
    g_mockSubscribeCommonEvent = mockRet;
}

int32_t MockUtils::MockGetSingleExtConfigFunc(int32_t index, stad::string &config)
{
    g_mockFunctionCallCount++;
    if (index == CLOUD_CONFIG_INDEX) {
        config = R"({"version" : "1.1.1.1"})";
        return ERR_OK;
    }
    return -1;
}
}
}  // namespace OHOS
