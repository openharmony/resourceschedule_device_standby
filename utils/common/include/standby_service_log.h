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

#include <string>

#include "hilog/log.h"

namespace OHOS {
namespace DevStandbyMgr {
#ifndef STANDBYSERVICE_LOG_DOMAIN
#define STANDBYSERVICE_LOG_DOMAIN 0xD001718
#endif

#ifndef STANDBYSERVICE_LOG_TAG
#define STANDBYSERVICE_LOG_TAG "StandbyService"
#endif

#ifdef __aarch64__
#define SPUBI64  "%{public}ld"
#define SPUBSize "%{public}lu"
#define SPUBU64  "%{public}lu"
#else
#define SPUBI64  "%{public}lld"
#define SPUBSize "%{public}u"
#define SPUBU64  "%{public}llu"
#endif

enum class StandbyServiceLogLevel : uint8_t { DEBUG = 0, INFO, WARN, ERROR, FATAL };

static constexpr OHOS::HiviewDFX::HiLogLabel STANDBYSERVICE_LABEL = {LOG_CORE, STANDBYSERVICE_LOG_DOMAIN,
    STANDBYSERVICE_LOG_TAG};

class StandbyServiceLog {
public:
    StandbyServiceLog() = delete;
    ~StandbyServiceLog() = delete;

    static bool JudgeLevel(const StandbyServiceLogLevel& level);

    static void SetLogLevel(const StandbyServiceLogLevel& level)
    {
        level_ = level;
    }

    static const StandbyServiceLogLevel& GetLogLevel()
    {
        return level_;
    }

    static std::string GetCurrFileName(const char *str);

private:
    static StandbyServiceLogLevel level_;
};

#define STANDBYSERVICE_PRINT_LOG(LEVEL, Level, fmt, ...)                                         \
    if (StandbyServiceLog::JudgeLevel(StandbyServiceLogLevel::LEVEL))     \
    OHOS::HiviewDFX::HiLog::Level(STANDBYSERVICE_LABEL,                        \
        "[%{public}s(%{public}s):%{public}d] " fmt,                                    \
        StandbyServiceLog::GetCurrFileName(__FILE__).c_str(),                            \
        __FUNCTION__,                                                                  \
        __LINE__,                                                                      \
        ##__VA_ARGS__)

#define STANDBYSERVICE_LOGD(fmt, ...) STANDBYSERVICE_PRINT_LOG(DEBUG, Debug, fmt, ##__VA_ARGS__)
#define STANDBYSERVICE_LOGI(fmt, ...) STANDBYSERVICE_PRINT_LOG(INFO, Info, fmt, ##__VA_ARGS__)
#define STANDBYSERVICE_LOGW(fmt, ...) STANDBYSERVICE_PRINT_LOG(WARN, Warn, fmt, ##__VA_ARGS__)
#define STANDBYSERVICE_LOGE(fmt, ...) STANDBYSERVICE_PRINT_LOG(ERROR, Error, fmt, ##__VA_ARGS__)
#define STANDBYSERVICE_LOGF(fmt, ...) STANDBYSERVICE_PRINT_LOG(FATAL, Fatal, fmt, ##__VA_ARGS__)
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_INCLUDE_STANDBY_SERVICE_LOG_H