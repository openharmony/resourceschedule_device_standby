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

#include <string>

#include "device_standby_switch.h"
#include "common_constant.h"
#include "standby_config_manager.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
void InitStandyMode()
{
    int32_t mode = 0;
    mode = StandbyConfigManager::GetInstance()->GetStandbyParam(DEVICE_STANGDY_MODE);
    if (mode > 0 && mode <= MODE_MAX_NUM) {
        STANDBT_MODE = (StandbyMode)mode;
    } else {
        STANDBT_MODE = StandbyMode::PHONEMODE;
    }
    STANDBYSERVICE_LOGI("StandBy mode: %{public}d ", STANDBT_MODE);
}

StandbyMode STANDBT_MODE = StandbyMode::UNKNOWN;
}  // namespace DevStandbyMgr
}  // namespace OHOS