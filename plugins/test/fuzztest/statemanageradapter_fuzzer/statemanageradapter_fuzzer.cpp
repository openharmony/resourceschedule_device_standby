/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "statemanageradapter_fuzzer.h"
#include "securec.h"
#include "state_manager_adapter.h"
#include "common_event_support.h"
#include "standby_service_impl.h"
#include "working_state.h"


namespace OHOS {
namespace DevStandbyMgr {
    constexpr size_t U32_AT_SIZE = 4;
    const std::string DUMP_D = "-D";
    const std::string DUMP_E = "-E";
    const std::string DUMP_S = "-S";
    const std::string DUMP_HALF_HOUR = "--halfhour";
    const std::string DUMP_MOTION = "--motion";
    const std::string DUMP_BLOCKED = "--blocked";
    const std::string DUMP_WORK = "working";
    const std::string ARGS_TEST_MODEM = "-M";
    const std::string LID_OPEN = "LID_OPEN";
    const std::string LID_CLOSE = "LID_CLOSE";
    const std::string STR_TEST = "test";
    const std::string TEST_TRUE = "true";
    const std::string TEST_FALSE = "false";
    const std::string TEST_BLANK = "";
    bool g_initFlag = false;
    const uint8_t *g_baseFuzzData = nullptr;
    size_t g_baseFuzzSize = 0;
    size_t g_baseFuzzPos;

    template <class T> T GetData()
    {
        T object{};
        size_t objectSize = sizeof(object);
        if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
            return object;
        }
        errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
        if (ret != EOK) {
            return {};
        }
        g_baseFuzzPos += objectSize;
        return object;
    }

    void PreciseCoverageHandleEvent()
    {
        std::shared_ptr<IStateManagerAdapter> stateManager =
            StandbyServiceImpl::GetInstance()->GetStateManager();
        StandbyMessage standbyMessageCase1{StandbyMessageType::COMMON_EVENT,
            EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED};
        StandbyMessage standbyMessageCase2{StandbyMessageType::RES_CTRL_CONDITION_CHANGED,
            EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON};
        StandbyMessage standbyMessageCase3{StandbyMessageType::RES_CTRL_CONDITION_CHANGED,
            EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING};
        StandbyMessage standbyMessageCase4{StandbyMessageType::RES_CTRL_CONDITION_CHANGED,
            EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF};
        StandbyMessage standbyMessageCase5{StandbyMessageType::RES_CTRL_CONDITION_CHANGED,
            EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING};
        StandbyMessage standbyMessageCase6{StandbyMessageType::RES_CTRL_CONDITION_CHANGED,
            EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED};
        StandbyMessage standbyMessageCase7{StandbyMessageType::RES_CTRL_CONDITION_CHANGED, LID_OPEN};
        StandbyMessage standbyMessageCase8{StandbyMessageType::RES_CTRL_CONDITION_CHANGED, LID_CLOSE};
        StandbyMessage standbyMessageCase9{StandbyMessageType::COMMON_EVENT,
            EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF};
        StandbyMessage standbyMessageCase10{StandbyMessageType::COMMON_EVENT,
            EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON};
        StandbyMessage standbyMessageCase11{StandbyMessageType::COMMON_EVENT,
            EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING};
        StandbyMessage standbyMessageCase12{StandbyMessageType::COMMON_EVENT,
            EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED};
        StandbyMessage standbyMessageCase13{StandbyMessageType::COMMON_EVENT, STR_TEST};
        StandbyMessage standbyMessageCase14{StandbyMessageType::COMMON_EVENT, LID_OPEN};
        StandbyMessage standbyMessageCase15{StandbyMessageType::COMMON_EVENT, LID_CLOSE};
        stateManager->HandleEvent(standbyMessageCase1);
        stateManager->HandleEvent(standbyMessageCase2);
        stateManager->HandleEvent(standbyMessageCase3);
        stateManager->HandleEvent(standbyMessageCase4);
        stateManager->HandleEvent(standbyMessageCase5);
        stateManager->HandleEvent(standbyMessageCase6);
        stateManager->HandleEvent(standbyMessageCase7);
        stateManager->HandleEvent(standbyMessageCase8);
        stateManager->HandleEvent(standbyMessageCase9);
        stateManager->HandleEvent(standbyMessageCase10);
        stateManager->HandleEvent(standbyMessageCase11);
        stateManager->HandleEvent(standbyMessageCase12);
        stateManager->HandleEvent(standbyMessageCase13);
        stateManager->HandleEvent(standbyMessageCase14);
        stateManager->HandleEvent(standbyMessageCase15);
    }

    void PreciseCoverage()
    {
        if (g_initFlag) {
            return;
        }
        g_initFlag = true;
        StandbyServiceImpl::GetInstance()->Init();
        std::shared_ptr<IStateManagerAdapter> stateManager =
            StandbyServiceImpl::GetInstance()->GetStateManager();
        stateManager->isEvalution_ = false;
        stateManager->StopEvalution();
        stateManager->Init();
        PreciseCoverageHandleEvent();
        const std::vector<std::string> argsInStr01 = {DUMP_D, DUMP_RESET_STATE};
        const std::vector<std::string> argsInStr02 = {DUMP_E};
        const std::vector<std::string> argsInStr03 = {DUMP_E, TEST_BLANK, TEST_FALSE};
        const std::vector<std::string> argsInStr04 = {DUMP_E, DUMP_WORK, TEST_TRUE};
        const std::vector<std::string> argsInStr05 = {DUMP_S, DUMP_MOTION};
        const std::vector<std::string> argsInStr06 = {DUMP_S, DUMP_BLOCKED};
        const std::vector<std::string> argsInStr07 = {DUMP_S, DUMP_HALF_HOUR};
        std::string result = "";
        stateManager->ShellDump(argsInStr01, result);
        stateManager->ShellDump(argsInStr02, result);
        stateManager->ShellDump(argsInStr03, result);
        stateManager->ShellDump(argsInStr04, result);
        stateManager->ShellDump(argsInStr05, result);
        stateManager->ShellDump(argsInStr06, result);
        stateManager->ShellDump(argsInStr07, result);
        stateManager->GetCurState();
        stateManager->GetPreState();
        std::shared_ptr<BaseState> workingStatePrt_ = std::make_shared<WorkingState>(StandbyState::WORKING, 0,
            StandbyServiceImpl::GetInstance()->GetStateManager(), StandbyServiceImpl::GetInstance()->GetHandler());
        workingStatePrt_->CheckTransitionValid(StandbyState::MAINTENANCE);
        workingStatePrt_->EndEvalCurrentState(true);
        workingStatePrt_->EndEvalCurrentState(false);
        stateManager->OnScreenOffHalfHour(false, true);
        stateManager->isEvalution_ = true;
        stateManager->EndEvalCurrentState(false);
        stateManager->isEvalution_ = false;
        stateManager->EndEvalCurrentState(true);
        stateManager->UnInit();
        stateManager->Init();
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
    {
        g_baseFuzzData = data;
        g_baseFuzzSize = size;
        g_baseFuzzPos = 0;
        uint32_t  uintParam = GetData<uint32_t>();
        std::string strParam((const char *) g_baseFuzzData + g_baseFuzzPos, g_baseFuzzSize - g_baseFuzzPos);
        StandbyMessage standbyMessage{uintParam, strParam};
        PreciseCoverage();
        StandbyServiceImpl::GetInstance()->GetStateManager()->HandleEvent(standbyMessage);
        return true;
    }
} // namespace DevStandbyMgr
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::DevStandbyMgr::U32_AT_SIZE) {
        return 0;
    }
    OHOS::DevStandbyMgr::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
