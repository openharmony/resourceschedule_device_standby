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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_HELPER_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif // STANDBY_SERVICE_UNIT_TEST

#include <memory>
#include <vector>

#include "continuous_task_callback_info.h"
#include "transient_task_app_info.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "singleton.h"
#include "standby_service_errors.h"

namespace OHOS {
namespace DevStandbyMgr {
using namespace OHOS::BackgroundTaskMgr;
class BackgroundTaskHelper : public std::enable_shared_from_this<BackgroundTaskHelper> {
DECLARE_DELAYED_SINGLETON(BackgroundTaskHelper);
public:
    static std::shared_ptr<BackgroundTaskHelper> GetInstance();
    /**
     * @brief Get all running continuous task info.
     */
    bool GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list);

    /**
     * @brief Get all running transient task info.
     */
    bool GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list);

private:
    BackgroundTaskHelper(const BackgroundTaskHelper&) = delete;
    BackgroundTaskHelper& operator= (const BackgroundTaskHelper&) = delete;
    BackgroundTaskHelper(BackgroundTaskHelper&&) = delete;
    BackgroundTaskHelper& operator= (BackgroundTaskHelper&&) = delete;
};
}
}
#endif // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_HELPER_H