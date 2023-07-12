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
#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_ISTRATEGY_MANAGER_ADAPTER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_ISTRATEGY_MANAGER_ADAPTER_H

#include <vector>
#include <memory>
#include <string>
#include <set>

#include "ibase_strategy.h"
#include "standby_service_errors.h"

namespace OHOS {
namespace DevStandbyMgr {
class IStrategyManagerAdapter {
public:
    virtual ~IStrategyManagerAdapter() = default;
    virtual bool Init() = 0;
    virtual bool UnInit() = 0;
    virtual void HandleEvent(const StandbyMessage& message) = 0;
    virtual void ShellDump(const std::vector<std::string>& argsInStr, std::string& result) = 0;
protected:
    virtual void RegisterPolicy(const std::vector<std::string>& strategies) = 0;
protected:
    std::vector<std::shared_ptr<IBaseStrategy>> strategyList_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_ISTRATEGY_MANAGER_ADAPTER_H
