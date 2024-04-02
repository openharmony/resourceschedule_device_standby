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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_ILISTENER_MANAGER_ADAPTER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_ILISTENER_MANAGER_ADAPTER_H

#include <vector>
#include <memory>
#include <map>

#include "standby_service_errors.h"
#include "imessage_listener.h"
#include "istate_manager_adapter.h"

namespace OHOS {
namespace DevStandbyMgr {
class IListenerManagerAdapter {
public:
    virtual bool Init() = 0;
    virtual bool UnInit() = 0;
    virtual ErrCode StartListener() = 0;
    virtual ErrCode StopListener() = 0;
    virtual void HandleEvent(const StandbyMessage& message) = 0;
    virtual void ShellDump(const std::vector<std::string>& argsInStr, std::string& result) = 0;
    virtual ~IListenerManagerAdapter() = default;
protected:
    std::vector<std::shared_ptr<IMesssageListener>> messageListenerList_ {};
    std::map<int32_t, std::shared_ptr<IMesssageListener>> listenerPluginMap_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_ILISTENER_MANAGER_ADAPTER_H
