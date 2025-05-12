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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_ALLOW_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_ALLOW_INFO_H

#include <string>
#include <memory>

#include "parcel.h"
namespace OHOS {
namespace DevStandbyMgr {
class AllowInfo : public Parcelable {
public:
    AllowInfo() = default;
    AllowInfo(uint32_t allowType, const std::string& name, int32_t duration) :allowType_(allowType),
        name_(name), duration_(duration) {}

    /**
     * @brief Unmarshals a purpose from a Parcel.
     *
     * @param parcel Indicates the parcel object for unmarshalling.
     * @return The info of delay suspend.
     */
    static AllowInfo *Unmarshalling(Parcel& in);

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel& out) const override;

    /**
     * @brief Get the allow type used to represent resource type.
     *
     * @return the allow type.
     */
    inline uint32_t GetAllowType() const
    {
        return allowType_;
    }

    /**
     * @brief Get the app name.
     *
     * @return the name of the app.
     */
    inline std::string GetName() const
    {
        return name_;
    }

    /**
     * @brief Get the duration.
     *
     * @return the duration.
     */
    inline int32_t GetDuration() const
    {
        return duration_;
    }

    /**
     * @brief Set the allow type which represents the resources.
     *
     * @param allowType represents allow type.
     */
    inline void SetAllowType(uint32_t allowType)
    {
        allowType_ = allowType;
    }

    /**
     * @brief Set the name of the allowed native process or apps.
     *
     * @param name the name of the allowed object.
     */
    inline void SetName(const std::string& name)
    {
        name_ = name;
    }

    /**
     * @brief Set the Duration object.
     *
     * @param duration timeOut.
     */
    inline void SetDuration(int32_t duration)
    {
        duration_ = duration;
    }
private:
    bool ReadFromParcel(Parcel& in);

    uint32_t allowType_;
    std::string name_;
    int32_t duration_;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_ALLOW_INFO_H