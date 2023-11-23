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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_IPC_INTERFACE_CODE_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_IPC_INTERFACE_CODE_H

#include <ipc_types.h>

/* SAID: 1914 */
namespace OHOS {
namespace DevStandbyMgr {
    enum class IStandbyInterfaceCode {
        SUBSCRIBE_STANDBY_CALLBACK = MIN_TRANSACTION_ID,
        UNSUBSCRIBE_STANDBY_CALLBACK,
        APPLY_ALLOW_RESOURCE,
        UNAPPLY_ALLOW_RESOURCE,
        GET_ALLOW_LIST,
        IS_DEVICE_IN_STANDBY,
        REPORT_WORK_SCHEDULER_STATUS,
        GET_RESTRICT_LIST,
        IS_STRATEGY_ENABLED,
        REPORT_DEVICE_STATE_CHANGED,
        HANDLE_EVENT
    };
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_IPC_INTERFACE_CODE_H