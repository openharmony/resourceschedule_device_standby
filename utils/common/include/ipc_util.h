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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_IPC_UTIL_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_IPC_UTIL_H

#include <iremote_object.h>
#include <iservice_registry.h>
#include <string_ex.h>

namespace OHOS {
namespace DevStandbyMgr {
#define READ_PARCEL(in, type, data)                         \
    do {                                                    \
        if (!(in).Read##type(data)) {                       \
            STANDBYSERVICE_LOGE("Failed to read "#data);            \
            return;                                         \
        }                                                   \
    } while (0)

#define READ_PARCEL_WITH_RET(in, type, data, ret)           \
    do {                                                    \
        if (!(in).Read##type(data)) {                       \
            STANDBYSERVICE_LOGE("Failed to read "#data);            \
            return (ret);                                   \
        }                                                   \
    } while (0)

#define WRITE_PARCEL(out, type, data)                       \
    do {                                                    \
        if (!(out).Write##type(data)) {                     \
            STANDBYSERVICE_LOGE("Failed to write "#data);           \
            return;                                         \
        }                                                   \
    } while (0)

#define WRITE_PARCEL_WITH_RET(out, type, data, ret)         \
    do {                                                    \
        if (!(out).Write##type(data)) {                     \
            STANDBYSERVICE_LOGE("Failed to write "#data);           \
            return (ret);                                   \
        }                                                   \
    } while (0)

};  // namespace DevStandbyMgr
};  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_IPC_UTIL_H
