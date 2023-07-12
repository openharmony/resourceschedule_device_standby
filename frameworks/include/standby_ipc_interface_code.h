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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_IPC_INTERFACE_CODE_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_IPC_INTERFACE_CODE_H

#include <ipc_types.h>

/* SAID: 1914 */
namespace OHOS {
namespace DevStandbyMgr {
    enum class StandbySubscriberInterfaceCode {
        ON_DEVICE_IDLE_MODE = FIRST_CALL_TRANSACTION,
        ON_ALLOW_LIST_CHANGED,
    };
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_IPC_INTERFACE_CODE_H