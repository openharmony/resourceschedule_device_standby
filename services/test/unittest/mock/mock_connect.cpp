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

#include "timed_task.h"
#include "ability_manager_helper.h"
#include "app_mgr_helper.h"
#include "bundle_manager_helper.h"
#include "common_event_observer.h"
#ifdef ENABLE_BACKGROUND_TASK_MGR
#include "background_task_helper.h"
#endif

namespace OHOS {
namespace DevStandbyMgr {
bool AppMgrHelper::Connect()
{
    return false;
}

bool BundleManagerHelper::Connect()
{
    bundleMgr_ = nullptr;
    return false;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS