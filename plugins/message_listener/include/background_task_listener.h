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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_LISTENER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_LISTENER_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif // STANDBY_SERVICE_UNIT_TEST

#include <memory>
#include <vector>

#include "background_task_mgr_helper.h"
#include "background_task_subscriber.h"
#include "imessage_listener.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "istate_manager_adapter.h"
#include "resource_callback_info.h"
#include "single_instance.h"
#include "standby_service_errors.h"

namespace OHOS {
namespace DevStandbyMgr {
using namespace OHOS::BackgroundTaskMgr;
class BackgroundTaskListener : public std::enable_shared_from_this<BackgroundTaskListener>, public IMesssageListener {
public:
    BackgroundTaskListener();
    ~BackgroundTaskListener() override = default;

    /**
     * @brief Register background task observer.
     */
    ErrCode StartListener() override;

    /**
     * @brief Unregister background task observer.
     */
    ErrCode StopListener() override;
private:
    class BgTaskListenerImpl : public OHOS::BackgroundTaskMgr::BackgroundTaskSubscriber {
    public:
        BgTaskListenerImpl();

        /**
        * Called back when a continuous task start.
        *
        * @param info Transient task app info.
        */
        virtual void OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override;

        /**
        * Called back when the app does not have transient task.
        *
        * @param info App info transient task .
        */
        virtual void OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override;

        /**
        * Called back when a continuous task start.
        *
        * @param info Continuous task app info.
        */
        virtual void OnContinuousTaskStart(
            const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;

        /**
        * Called back when a continuous task end.
        *
        * @param info Continuous task info.
        */
        virtual void OnContinuousTaskStop(
            const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;

    private:
        /**
         * @brief dispatch BG_TASK_STATUS_CHANGE event when background task started or stopped
         */
        void OnTaskStatusChanged(const std::string& type, bool started, int32_t uid, int32_t pid,
            const std::string& bundleName);
    private:
        std::shared_ptr<AppExecFwk::EventHandler> handler_ {nullptr};
    };

private:
    std::unique_ptr<BackgroundTaskSubscriber> bgTaskListenerImpl_ {nullptr};
};
}
}
#endif // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_LISTENER_H