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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_LISTENER_MANAGER_ADAPTER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_LISTENER_MANAGER_ADAPTER_H

#include "ilistener_manager_adapter.h"
#include "standby_service_errors.h"

namespace OHOS {
namespace DevStandbyMgr {
class ListenerManagerAdapter : public IListenerManagerAdapter {
public:
    ~ListenerManagerAdapter() override = default;
    bool Init() override;
    bool UnInit() override;
    void HandleEvent(const StandbyMessage& message) override;
    ErrCode StartListener() override;
    ErrCode StopListener() override;
    void ShellDump(const std::vector<std::string>& argsInStr, std::string& result) override;
private:
    void UpdateListenerList(const StandbyMessage& message);
    void AddSystemServiceListener(int32_t systemAbilityId);
    void RemoveSystemServiceListener(int32_t systemAbilityId);
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_LISTENER_MANAGER_ADAPTER_H
