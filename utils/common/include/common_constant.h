/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_COMMON_CONSTANT_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_COMMON_CONSTANT_H

#include <string>
#include <vector>
#include <map>

namespace OHOS {
namespace DevStandbyMgr {
extern const std::string TRANSIT_NEXT_STATE_TIMED_TASK;
extern const std::string TRANSIT_NEXT_STATE_CONDITION_TASK;
extern const std::string TRANSIT_NEXT_PHASE_INSTANT_TASK;
extern const std::string REPEATED_MOTION_DETECTION_TASK;

extern const std::string DEFAULT_PLUGIN_NAME;

extern const std::string DETECT_MOTION_CONFIG;
extern const int32_t REPEATED_MOTION_DETECTION_INTERVAL;
extern const std::string DARK_TIMEOUT;
extern const std::string NAP_TIMEOUT;
extern const std::string NAP_MAINT_DURATION;

extern const std::string SLEEP_MAINT_DURATOIN;
extern const std::string NAP_MAINTENANCE_TIMEOUT;
extern const std::string SLEEP_MAINT_TIMEOUT;
extern const std::vector<std::string> STATE_NAME_LIST;

extern const std::string MOTION_THREADSHOLD;

extern const std::string MOTION_DECTION_TASK;
extern const int32_t MOTION_DETECTION_TIMEOUT;
extern const int32_t REST_TIMEOUT;
extern const int32_t TOTAL_TIMEOUT;
extern const int32_t PERIODLY_TASK_DECTION_TIMEOUT;
extern const int32_t PERIODLY_TASK_REST_TIMEOUT;
extern const int32_t PERIODLY_TASK_TOTAL_TIMEOUT;
extern const int64_t SENSOR_SAMPLING_RATE;
extern const int64_t SENSOR_REPORTING_RATE;
extern const int64_t HALF_HOUR;

extern const std::string NAP_SWITCH;
extern const std::string SLEEP_SWITCH;
extern const std::string S3_SWITCH;
extern const std::string DEVICE_STANGDY_MODE;

extern const std::string PREVIOUS_STATE;
extern const std::string CURRENT_STATE;
extern const std::string PREVIOUS_PHASE;
extern const std::string CURRENT_PHASE;
extern const std::string SENSOR_DETECTION;
extern const std::string RES_CTRL_CONDITION;
extern const std::string SCR_OFF_HALF_HOUR_STATUS;
extern const std::string NET_IDLE_POLICY_STATUS;
extern const std::string BG_TASK_TYPE;
extern const std::string BG_TASK_STATUS;
extern const std::string BG_TASK_UID;
extern const std::string BG_TASK_PID;
extern const std::string BG_TASK_BUNDLE_NAME;
extern const std::string BG_TASK_RESOURCE_STATUS;
extern const std::string SA_STATUS;
extern const std::string SA_ID;

extern const std::string CONTINUOUS_TASK;
extern const std::string TRANSIENT_TASK;
extern const std::string WORK_SCHEDULER;

extern const std::string LID_CLOSE;
extern const std::string LID_OPEN;

extern const uint32_t NO_DUMP_PARAM_NUMS;
extern const uint32_t DUMP_DETAILED_INFO_MAX_NUMS;
extern const uint32_t DUMP_SLEEP_SWITCH_PARAM_NUMS;
extern const uint32_t DUMP_SLEEP_ENTER_STATE_NUMS;
extern const uint32_t DUMP_SLEEP_ALLOW_LIST_NUMS;
extern const uint32_t DUMP_SLEEP_APPLY_ALLOW_LIST_NUMS;
extern const uint32_t DUMP_SWITCH_PARAM_NUMS;
extern const uint32_t DUMP_STATE_TIMEOUT_PARAM_NUMS;

extern const std::string DUMP_ON;
extern const std::string DUMP_OFF;
extern const std::string DUMP_SLEEP_MODE;
extern const std::string DUMP_DAYTIME_SLEEP_MODE;
extern const std::string DUMP_NIGHTTIME_SLEEP_MODE;
extern const std::string DUMP_DEBUG_SWITCH;
extern const std::string DUMP_RESET_STATE;
extern const std::string DUMP_DETAIL_CONFIG;
extern const std::string DUMP_STRATGY_DETAIL;
extern const std::string DUMP_POWEROFF_STRATEGY;
extern const std::string DUMP_POWERSAVE_FIREWALL;

extern const std::string DUMP_DETAIL_INFO;
extern const std::string DUMP_ENTER_STATE;
extern const std::string DUMP_APPLY_ALLOW_RECORD;
extern const std::string DUMP_SIMULATE_SENSOR;
extern const std::string DUMP_SUBSCRIBER_OBSERVER;
extern const std::string DUMP_TURN_ON_OFF_SWITCH;
extern const std::string DUMP_CHANGE_STATE_TIMEOUT;
extern const std::string DUMP_PUSH_STRATEGY_CHANGE;
extern const int32_t DUMP_FIRST_PARAM;
extern const int32_t DUMP_SECOND_PARAM;
extern const int32_t DUMP_THIRD_PARAM;
extern const int32_t DUMP_FOURTH_PARAM;
extern const int32_t DUMP_FIFTH_PARAM;
extern const int32_t DUMP_SIXTH_PARAM;
extern const int32_t DUMP_SEVENTH_PARAM;

extern const int32_t NIGHT_ENTRANCE_HOUR;
extern const int32_t NIGHT_ENTRANCE_MIN;
extern const int32_t DAY_ENTRANCE_HOUR;
extern const int32_t DAY_ENTRANCE_MIN;
extern const std::string AIRPLANE_MODE_PARAMETER;
}  // namespace DevStandbyMgr
}  // namespace OHOS

#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_UTILS_COMMON_CONSTANT_H