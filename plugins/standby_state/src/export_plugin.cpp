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

#include "listener_manager_adapter.h"
#include "constraint_manager_adapter.h"
#include "strategy_manager_adapter.h"
#include "state_manager_adapter.h"
#include "standby_service_impl.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
extern "C" bool OnPluginRegister()
{
    IConstraintManagerAdapter* constraintManager = new ConstraintManagerAdapter();
    IListenerManagerAdapter* listenerManager = new ListenerManagerAdapter();
    IStrategyManagerAdapter* strategyManager = new StrategyManagerAdapter();
    IStateManagerAdapter* stateManager = new StateManagerAdapter();
    if (!constraintManager || !listenerManager || !strategyManager || !stateManager) {
        STANDBYSERVICE_LOGW("plugin is nullptr, failed to load valid plugin");
        delete constraintManager;
        delete listenerManager;
        delete strategyManager;
        delete stateManager;
        return false;
    }
    StandbyServiceImpl::GetInstance()->RegisterPluginInner(constraintManager,
        listenerManager, strategyManager, stateManager);
    STANDBYSERVICE_LOGI("plugin register succeed");
    return true;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS