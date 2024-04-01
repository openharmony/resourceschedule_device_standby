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

#include "background_task_helper.h"
#include "background_task_mgr_helper.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
IMPLEMENT_SINGLE_INSTANCE(BackgroundTaskHelper);

BackgroundTaskHelper::BackgroundTaskHelper()
{
}

BackgroundTaskHelper::~BackgroundTaskHelper()
{
}

bool WEAK_FUNC BackgroundTaskHelper::GetContinuousTaskApps(
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    #ifdef ENABLE_BACKGROUND_TASK_MGR
    if (BackgroundTaskMgrHelper::GetContinuousTaskApps(list) != OHOS::ERR_OK) {
        STANDBYSERVICE_LOGW("Get continuous task failed");
        return false;
    }
    #endif
    return true;
}

bool WEAK_FUNC BackgroundTaskHelper::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    #ifdef ENABLE_BACKGROUND_TASK_MGR
    if (BackgroundTaskMgrHelper::GetTransientTaskApps(list) != OHOS::ERR_OK) {
        STANDBYSERVICE_LOGE("Get transient task failed");
        return false;
    }
    #endif
    return true;
}
} // OHOS
} // DevStandbyMgr