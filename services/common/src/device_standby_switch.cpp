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

#include "device_standby_switch.h"

#include <string>

namespace OHOS {
namespace DevStandbyMgr {
const std::string KEY_BUILD_CHARACTER = "const.product.devicetype";

DeviceType GetDeviceType()
{
    std::string deviceType = system::GetParameter(KEY_BUILD_CHARACTER, "unknown");
    if (deviceType == "phone" || deviceType == "default") {
        return DeviceType::PHONE;
    } else if (deviceType == "tablet" || deviceType == "2in1") {
        return DeviceType::TABLET;
    } else if (deviceType == "watch") {
        return DeviceType::WATCH;
    } else if (deviceType == "tv") {
        return DeviceType::TV;
    } else if (deviceType == "car") {
        return DeviceType::IVI;
    }
    return DeviceType::UNKNOWN;
}

const DeviceType DEVICE_TYPE = GetDeviceType();
}  // namespace DevStandbyMgr
}  // namespace OHOS