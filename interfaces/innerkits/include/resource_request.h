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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_RESOURCE_TYPE_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_RESOURCE_TYPE_H

#include <string>
#include <memory>

#include "parcel.h"
namespace OHOS {
namespace DevStandbyMgr {
struct ReasonCodeEnum {
    enum : uint32_t {
        REASON_NATIVE_API = 0,
        REASON_APP_API = 1,
    };
};
class ResourceRequest : public Parcelable {
public:
    ResourceRequest() = default;
    ResourceRequest(uint32_t allowType, int32_t uid, const std::string& name, int32_t duration,
        const std::string& reason, uint32_t reasonCode) : allowType_(allowType), uid_(uid), name_(name),
        duration_(duration), reason_(reason), reasonCode_(reasonCode) {}

    /**
     * @brief Unmarshals a purpose from a Parcel.
     *
     * @param parcel Indicates the parcel object for unmarshalling.
     * @return The info of delay suspend.
     */
    static ResourceRequest* Unmarshalling(Parcel& in);

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel& out) const override;

    /**
     * @brief Get the uid of the resource request.
     *
     * @return the allow type.
     */
    inline uint32_t GetAllowType() const
    {
        return allowType_;
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
     * @brief Get the uid of the resource request.
     *
     * @return the uid.
     */
    inline int32_t GetUid() const
    {
        return uid_;
    }

    /**
     * @brief Set the uid of the resource request.
     *
     * @param uid
     */
    inline void SetUid(int32_t uid)
    {
        uid_ = uid;
    }

    /**
     * @brief Get the name of the resource request.
     *
     * @return name of the resource request.
     */
    inline std::string GetName() const
    {
        return name_;
    }

    /**
     * @brief Set the name of the resource request.
     *
     * @param name name of the resource request.
     */
    inline void SetName(const std::string& name)
    {
        name_ = name;
    }

    /**
     * @brief Get the duration of the resource request.
     *
     * @return the duration of the resource request.
     */
    inline int32_t GetDuration() const
    {
        return duration_;
    }

    /**
     * @brief Set the Duration of the resource request.
     *
     * @param duration timeOut of the resource request.
     */
    inline void SetDuration(int32_t duration)
    {
        duration_ = duration;
    }

    /**
     * @brief Get the reason object of the resource request.
     *
     * @return reason of the resource request.
     */
    inline std::string GetReason() const
    {
        return reason_;
    }

    /**
     * @brief Set the reason object of the resource request.
     *
     * @param reason reason of the resource request.
     */
    inline void SetReason(const std::string& reason)
    {
        reason_ = reason;
    }

    /**
     * @brief Get the reason code object of the resource request.
     *
     * @param reason reason code of the resource request.
     */
    inline uint32_t GetReasonCode() const
    {
        return reasonCode_;
    }

    /**
     * @brief Set the reason code object of the resource request.
     *
     * @param reason reason code of the resource request.
     */
    inline void SetReasonCode(uint32_t reasonCode)
    {
        reasonCode_ = reasonCode;
    }

private:
    bool ReadFromParcel(Parcel& in);

    uint32_t allowType_;
    int32_t uid_;
    std::string name_;
    int32_t duration_;
    std::string reason_;
    uint32_t reasonCode_;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_RESOURCE_TYPE_H
