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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_BASE_STATE_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_BASE_STATE_H

#include <memory>
#include <stdint.h>
#include <unordered_map>

#include "event_handler.h"
#ifdef STANDBY_POWER_MANAGER_ENABLE
#include "power_mgr_client.h"
#endif
#include "common_constant.h"
#include "standby_service_errors.h"
#include "standby_messsage.h"
#include "istrategy_manager_adapter.h"
#include "standby_state.h"
#include "timed_task.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    const uint32_t THREE_BYTES_LEN = 24;
    const uint32_t TWO_BYTES_LEN = 16;
    const uint32_t ONE_BYTE_LEN = 8;
}

struct ConstraintEvalParam {
    uint32_t curState_ {0};
    uint32_t curPhase_ {0};
    uint32_t nextState_ {0};
    uint32_t nextPhase_ {0};
    bool isRepeatedDetection_ {false};
    ConstraintEvalParam() = default;
    ConstraintEvalParam(uint32_t curState, uint32_t curPhase, uint32_t nextState, uint32_t nextPhase)
        : curState_(curState), curPhase_(curPhase), nextState_(nextState), nextPhase_(nextPhase) {}
    inline uint32_t GetHashValue() const
    {
        uint32_t res = (curState_ << THREE_BYTES_LEN) + (curPhase_ << TWO_BYTES_LEN) +
            (nextState_ << ONE_BYTE_LEN) + (nextPhase_ << 1);
        if (isRepeatedDetection_) {
            return res + 1;
        }
        return res;
    }
};

class IStateManagerAdapter;
struct StandbyMessage;
class BaseState {
public:
    BaseState(uint32_t curState, uint32_t curPhase,  const std::shared_ptr<IStateManagerAdapter>&
        stateManager, std::shared_ptr<AppExecFwk::EventHandler>& handler): curState_(curState),
        curPhase_(curPhase), stateManager_(stateManager), handler_(handler) {}
    virtual ~BaseState() = default;
    virtual ErrCode Init(const std::shared_ptr<BaseState>& statePtr);
    virtual ErrCode UnInit();
    virtual uint32_t GetCurState();
    virtual uint32_t GetCurInnerPhase();
    virtual ErrCode BeginState() = 0;
    virtual ErrCode EndState() = 0;

    virtual bool CheckTransitionValid(uint32_t nextState) = 0;
    virtual void EndEvalCurrentState(bool evalResult) = 0;
    virtual void StartTransitNextState(const std::shared_ptr<BaseState>& statePtr);
    virtual void TransitToPhase(uint32_t curPhase, uint32_t nextPhase);
    virtual void TransitToPhaseInner(uint32_t prePhase, uint32_t curPhase);
    virtual bool IsInFinalPhase();
    virtual void OnStateBlocked();
    virtual void ShellDump(const std::vector<std::string>& argsInStr, std::string& result);

    virtual void SetTimedTask(const std::string& timedTaskName, uint64_t timedTaskId);
    virtual ErrCode StartStateTransitionTimer(int64_t triggerTime);
    virtual ErrCode StopTimedTask(const std::string& timedTaskName);
    virtual void DestroyAllTimedTask();

    static void InitRunningLock();
    static void AcquireStandbyRunningLock();
    static void ReleaseStandbyRunningLock();
protected:
    uint32_t curState_ {0};
    uint32_t curPhase_ {0};
    uint32_t nextState_ {0};
    std::weak_ptr<IStateManagerAdapter> stateManager_ {};
    std::shared_ptr<AppExecFwk::EventHandler> &handler_;
    uint64_t enterStandbyTimerId_ {};
    std::unordered_map<std::string, uint64_t> timedTaskMap_ {};
    static bool runningLockStatus_;
    #ifdef STANDBY_POWER_MANAGER_ENABLE
    static std::shared_ptr<PowerMgr::RunningLock> standbyRunningLock_;
    #endif
};

class StateWithMaint {
protected:
    virtual int64_t CalculateMaintTimeOut(const std::shared_ptr<IStateManagerAdapter>&
        stateManagerPtr, bool isFirstInterval);
protected:
    int32_t maintIntervalIndex_ {0};
    std::vector<int32_t> maintInterval_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_BASE_STATE_H