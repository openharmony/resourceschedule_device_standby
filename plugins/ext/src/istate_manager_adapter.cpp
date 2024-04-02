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

#include "base_state.h"

#include "istate_manager_adapter.h"
#include "standby_messsage.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
void IStateManagerAdapter::SetEvalution(bool isEvalution)
{
    isEvalution_ = isEvalution;
}

bool IStateManagerAdapter::IsEvalution()
{
    return isEvalution_;
}

int64_t IStateManagerAdapter::GetScreenOffTimeStamp()
{
    return screenOffTimeStamp_;
}

bool IStateManagerAdapter::IsScreenOn()
{
    return isScreenOn_;
}

bool IStateManagerAdapter::IsScrOffHalfHourCtrl()
{
    return scrOffHalfHourCtrl_;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS