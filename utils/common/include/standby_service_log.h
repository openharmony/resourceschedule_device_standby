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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_INCLUDE_STANDBY_SERVICE_LOG_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_INCLUDE_STANDBY_SERVICE_LOG_H

#include "hilog/log.h"

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#endif
#define LOG_DOMAIN 0xD001718

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "StandbyService"

namespace OHOS {
namespace DevStandbyMgr {
#ifdef __aarch64__
#define SPUBI64  "%{public}ld"
#define SPUB_SIZE "%{public}lu"
#define SPUBU64  "%{public}lu"
#elif __x86_64__
#define SPUBI64  "%{public}ld"
#define SPUB_SIZE "%{public}lu"
#define SPUBU64  "%{public}lu"
#else
#define SPUBI64  "%{public}lld"
#define SPUB_SIZE "%{public}u"
#define SPUBU64  "%{public}llu"
#endif

#ifndef FILENAME
#define FILENAME  (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef FORMAT_LOG
#define FORMAT_LOG(fmt, ...) "[%{public}s(%{public}s):%{public}d] " fmt, FILENAME, __FUNCTION__, __LINE__, ##__VA_ARGS__
#endif

#define STANDBYSERVICE_PRINT_LOGD(fmt, ...)  HILOG_DEBUG(LOG_CORE, fmt, ##__VA_ARGS__)
#define STANDBYSERVICE_PRINT_LOGI(fmt, ...)  HILOG_INFO(LOG_CORE, fmt, ##__VA_ARGS__)
#define STANDBYSERVICE_PRINT_LOGW(fmt, ...)  HILOG_WARN(LOG_CORE, fmt, ##__VA_ARGS__)
#define STANDBYSERVICE_PRINT_LOGE(fmt, ...)  HILOG_ERROR(LOG_CORE, fmt, ##__VA_ARGS__)
#define STANDBYSERVICE_PRINT_LOGF(fmt, ...)  HILOG_FATAL(LOG_CORE, fmt, ##__VA_ARGS__)

#define STANDBYSERVICE_LOGD(...)  STANDBYSERVICE_PRINT_LOGD(FORMAT_LOG(__VA_ARGS__))
#define STANDBYSERVICE_LOGI(...)  STANDBYSERVICE_PRINT_LOGI(FORMAT_LOG(__VA_ARGS__))
#define STANDBYSERVICE_LOGW(...)  STANDBYSERVICE_PRINT_LOGW(FORMAT_LOG(__VA_ARGS__))
#define STANDBYSERVICE_LOGE(...)  STANDBYSERVICE_PRINT_LOGE(FORMAT_LOG(__VA_ARGS__))
#define STANDBYSERVICE_LOGF(...)  STANDBYSERVICE_PRINT_LOGF(FORMAT_LOG(__VA_ARGS__))
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_INCLUDE_STANDBY_SERVICE_LOG_H
