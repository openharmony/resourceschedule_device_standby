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

#include "network_strategy.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
void NetworkStrategy::HandleEvent(const StandbyMessage& message)
{
    STANDBYSERVICE_LOGD("NetworkStrategy revceived message %{public}u, action: %{public}s",
        message.eventId_, message.action_.c_str());
}

ErrCode NetworkStrategy::OnCreated()
{
    return ERR_OK;
}

ErrCode NetworkStrategy::OnDestroy()
{
    return ERR_OK;
}

void NetworkStrategy::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
}
}  // namespace DevStandbyMgr
}  // namespace OHOS