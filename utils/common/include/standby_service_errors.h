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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_INCLUDE_STANDBY_SERVICE_INNER_ERRORS_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_INCLUDE_STANDBY_SERVICE_INNER_ERRORS_H

#include <map>
#include <string>
#include "errors.h"

namespace OHOS {
namespace DevStandbyMgr {
/**
 * ErrCode layout
 *
 * +--+--+--+--+--+--+--+--+---+---+
 * |09|08|07|06|05|04|03|02| 01| 00|
 * +--+--+--+--+--+--+--+--+---+---+
 * | Syscap |      Code    |Subcode|
 * +--+--+--+--+--+--+--+--+---+---+
 */
const int OFFSET = 100;
const int THRESHOLD = 1000;
// StrandbyService Common Error Code Defined.
enum : int32_t {
    // errcode for common
    ERR_STANDBY_PERMISSION_DENIED = 201,
    ERR_STANDBY_NOT_SYSTEM_APP = 202,
    ERR_STANDBY_INVALID_PARAM = 401,

    ERR_STANDBY_NO_MEMORY = 980000101,
    ERR_STANDBY_PARCELABLE_FAILED = 980000201,
    ERR_STANDBY_TRANSACT_FAILED = 980000301,
    ERR_STANDBY_SYS_NOT_READY = 980000401,
    ERR_STANDBY_GET_SABILITY_FAILED,
    ERR_STANDBY_SERVICE_NOT_CONNECTED,
    ERR_STANDBY_DUMP_SAVE_DENIED,
    ERR_STANDBY_OBJECT_EXISTS = 980000501,
    ERR_STANDBY_OBJECT_NOT_EXIST,
    ERR_STANDBY_CONFIG_FILE_LOAD_FAILED = 980000601,
    ERR_STANDBY_CONFIG_FILE_PARSE_FAILED,

    ERR_STANDBY_OBSERVER_ALREADY_EXIST = 980000701,
    ERR_STANDBY_OBSERVER_INIT_FAILED,
    ERR_STANDBY_OBSERVER_RESET_FAILED,
    ERR_REGISTER_BACKGROUND_TASK_FAILED,
    ERR_UNREGISTER_BACKGROUND_TASK_FAILED,
    ERR_BGTASK_LISTENER_NULL,

    ERR_STANDBY_PLUGIN_NOT_EXIST = 980000801,
    ERR_STANDBY_PLUGIN_FUNC_NOT_EXIST,
    ERR_STANDBY_PLUGIN_NOT_AVAILABLE,
    ERR_STANDBY_PLUGIN_SINGLETON_NOT_EXIST,

    ERR_STATE_MANAGER_IS_NULLPTR = 980000901,
    ERR_STANDBY_STATE_INIT_FAILED,
    ERR_STANDBY_STATE_TRANSITION_FAILED,
    ERR_INTER_STATE_CONSTRAINT_NOT_PERMIT,
    ERR_STANDBY_STATAE_BLOCKED,
    ERR_STANDBY_STATE_TIMING_SEQ_ERROR,
    ERR_STANDBY_TIMER_SERVICE_ERROR,
    ERR_STANDBY_TIMERID_NOT_EXIST,
    ERR_STANDBY_CURRENT_STATE_NOT_MATCH,
    ERR_STANDBY_CURRENT_PHASE_NOT_MATCH,
    ERR_STANDBY_STRATEGY_STATE_REPEAT,
    ERR_STANDBY_GET_PROCESSNAME_INFO,
    ERR_STANDBY_STRATEGY_NOT_MATCH,
    ERR_STANDBY_STRATEGY_NOT_DEPLOY,
    ERR_STANDBY_START_LISENER_FAILED,
    ERR_STANDBY_STOP_LISENER_FAILED,
    ERR_STANDBY_START_SENSOR_FAILED,
    ERR_STANDBY_SEND_NOTIFICATION_FAILED,
    ERR_STANDBY_KEY_INFO_NOT_MATCH,
    ERR_STANDBY_REPAET_INIT,

    ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE = 980001001,
    ERR_STANDBY_LOAD_DUMP_FAILED,
    ERR_STANDBY_RESTRICTION_CONDITION_NOT_MATCH,
    RR_DATASHARE_OBJECT_NULLPTR,
    RR_DATASHARE_QUERY_FAILED,
};

enum ParamErr: int32_t {
    ERR_PARAM_NUMBER_ERR = 9800401,
    ERR_CALLBACK_NULL_OR_TYPE_ERR,
    ERR_REASON_NULL_OR_TYPE_ERR,
    ERR_RESOURCE_TYPES_INVALID,
    ERR_UID_INVALID,
    ERR_NAME_INVALID_OR_EMPTY,
    ERR_DURATION_INVALID,
    ERR_REASON_INVALID_TYPE_ERR,
};

inline std::map<int32_t, std::string> saErrCodeMsgMap = {
    {ERR_STANDBY_PERMISSION_DENIED, "Permission denied."},
    {ERR_STANDBY_NOT_SYSTEM_APP,
        "System API verification failed."},
    {ERR_STANDBY_NO_MEMORY, "Memory operation failed. Failed to allocate the memory."},
    {ERR_STANDBY_PARCELABLE_FAILED, "Parcel operation failed."},
    {ERR_STANDBY_TRANSACT_FAILED, "IPC failed."},
    {ERR_STANDBY_SYS_NOT_READY, "System service operation failed. The system service is not ready."},
    {ERR_STANDBY_SERVICE_NOT_CONNECTED, "System service operation failed. The system service is not connected."},
    {ERR_STANDBY_OBJECT_EXISTS,
        "Standby service verification failed. The application has subscribed standby callback."},
    {ERR_STANDBY_CONFIG_FILE_LOAD_FAILED,
        "Config file of standby service load failed, please check the path of the config file."},
    {ERR_STANDBY_CONFIG_FILE_PARSE_FAILED,
        "Config file of standby service load failed, please check the format and content of the config file."},
    {ERR_STANDBY_PLUGIN_NOT_EXIST, "Plugin of standby service doesn't exist."},
    {ERR_STANDBY_PLUGIN_FUNC_NOT_EXIST, "Export plugin function doesn't exist."},
    {ERR_STANDBY_PLUGIN_NOT_AVAILABLE, "Plugin of standby service is not available."},
    {ERR_STANDBY_PLUGIN_SINGLETON_NOT_EXIST, "The singleton object loaded from plugin doesn't exist."},
    {ERR_STATE_MANAGER_IS_NULLPTR, "The state manager adapter can not be nullptr."},
};

inline std::map<int32_t, std::string> paramErrCodeMsgMap = {
    {ERR_STANDBY_INVALID_PARAM, "The input param is invalid."},
    {ERR_PARAM_NUMBER_ERR, "The number of arguments is wrong."},
    {ERR_CALLBACK_NULL_OR_TYPE_ERR, "The callback cannot be null and its type must be function."},
    {ERR_RESOURCE_TYPES_INVALID, "The resourcesType cannot be null and must be integer greater than 0"\
        "and less than maximum value."},
    {ERR_UID_INVALID, "The uid can not less than 0"},
    {ERR_NAME_INVALID_OR_EMPTY, "The name must be valid string and can not be empty"},
    {ERR_DURATION_INVALID, "The duration must be valid integer and can not less than 0"},

    {ERR_REASON_INVALID_TYPE_ERR, "The reason cannot be null and its type must be string."},
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_INCLUDE_STANDBY_SERVICE_INNER_ERRORS_H