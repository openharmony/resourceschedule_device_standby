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
#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_STRATEGY_INCLUDE_TIMER_STRATEGY_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_STRATEGY_INCLUDE_TIMER_STRATEGY_H

#include "ibase_strategy.h"

namespace OHOS {
namespace DevStandbyMgr {
class TimerStrategy : public IBaseStrategy {
public:
    void HandleEvent(const StandbyMessage& message) override;
    ErrCode OnCreated() override;
    ErrCode OnDestroy() override;
    void ShellDump(const std::vector<std::string>& argsInStr, std::string& result) override;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_STRATEGY_INCLUDE_TIMER_STRATEGY_H