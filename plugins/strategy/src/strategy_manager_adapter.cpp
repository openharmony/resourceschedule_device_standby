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

#include "strategy_manager_adapter.h"

#include "ibase_strategy.h"
#include "standby_service_log.h"
#ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
#include "network_strategy.h"
#endif
#include "standby_config_manager.h"
#include "running_lock_strategy.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
const std::map<std::string, std::shared_ptr<IBaseStrategy>> strategyMap_ {
    #ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
    { "NETWORK", std::make_shared<NetworkStrategy>() },
    #endif
    { "RUNNING_LOCK", std::make_shared<RunningLockStrategy>() },
};
}

bool StrategyManagerAdapter::Init()
{
    if (StandbyConfigManager::GetInstance() == nullptr) {
        STANDBYSERVICE_LOGE("standby service ptr is nullptr, init failed");
        return false;
    }
    const auto& strategyConfigList = StandbyConfigManager::GetInstance()->GetStrategyConfigList();
    if (strategyConfigList.empty()) {
        STANDBYSERVICE_LOGI("strategies is disabled");
        return true;
    }
    RegisterPolicy(strategyConfigList);
    STANDBYSERVICE_LOGI("strategy manager plugin initialization succeed");
    return true;
}

bool StrategyManagerAdapter::UnInit()
{
    for (const auto& strategy : strategyList_) {
        strategy->OnDestroy();
    }
    strategyList_.clear();
    return true;
}

void StrategyManagerAdapter::RegisterPolicy(const std::vector<std::string>& strategies)
{
    for (const auto& item : strategies) {
        auto strategy = strategyMap_.find(item);
        if (strategy == strategyMap_.end()) {
            continue;
        }
        STANDBYSERVICE_LOGI("strategy manager init %{public}s", item.c_str());
        auto strategyPtr = strategy->second;
        if (!strategyPtr) {
            continue;
        }
        if (strategyPtr->OnCreated() == ERR_OK) {
            strategyList_.emplace_back(strategyPtr);
        }
    }
}

void StrategyManagerAdapter::HandleEvent(const StandbyMessage& message)
{
    STANDBYSERVICE_LOGD("StrategyManagerAdapter revceive message %{public}u, action: %{public}s",
        message.eventId_, message.action_.c_str());
    for (const auto &strategy : strategyList_) {
        strategy->HandleEvent(message);
    }
}

void StrategyManagerAdapter::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
    for (const auto &strategy : strategyList_) {
        strategy->ShellDump(argsInStr, result);
    }
}
}  // namespace DevStandbyMgr
}  // namespace OHOS