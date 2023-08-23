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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_CONSTRAINTS_INCLUDE_MOTION_SENSOR_CONSTRAINT_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_CONSTRAINTS_INCLUDE_MOTION_SENSOR_CONSTRAINT_H

#include <memory>
#include <map>
#include <functional>
#include "refbase.h"

#include "sensor_agent.h"
#include "sensor_agent_type.h"
#include "iconstraint_monitor.h"
#include "base_state.h"

namespace OHOS {
namespace DevStandbyMgr {
class IStateManagerAdapter;
class MotionSensorMonitor : public IConstraintMonitor,
    public std::enable_shared_from_this<MotionSensorMonitor> {
public:
    MotionSensorMonitor(int32_t detectionTimeOut, int32_t restTimeOut, int32_t totalTimeOut,
        const ConstraintEvalParam& params);
    bool Init() override;
    void StartMonitoring() override;
    void StopMonitoring() override;
    static double GetEnergy();
    static void SetEnergy(double energy);
    static void AddEnergy(AccelData *accelData);
    static void MotionSensorCallback(SensorEvent *event);
    static void AcceleromterCallback(SensorEvent *event);
    static void RepeatAcceleromterCallback(SensorEvent *event);

private:
    bool InitSensorUserMap(SensorInfo* sensorInfo, int32_t count);
    void AssignAcclerometerSensorCallBack();
    void AssignMotionSensorCallBack();
    ErrCode StartMonitoringInner();
    void StopMonitoringInner();
    bool CheckSersorUsable(SensorInfo *sensorInfo, int32_t count, int32_t sensorTypeId);
    ErrCode StartSensor();
    void StopSensor();
    void PeriodlyStartMotionDetection();
    void StopMotionDetection();

private:
    const int32_t detectionTimeOut_;
    const int32_t restTimeOut_;
    const int32_t totalTimeOut_;

    static double energy_;
    static bool hasPrevAccelData_;
    static AccelData previousAccelData_;
    static AccelData currentAccelData_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {};
    ConstraintEvalParam params_{};
    bool isMonitoring_ {false};

    // key is sensor type, value is sensorUser with callback function pointer
    std::map<int32_t, SensorUser> sensorUserMap_ {};
};
} // DevStandbyMgr
} // OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_CONSTRAINTS_INCLUDE_MOTION_SENSOR_CONSTRAINT_H
