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

#include "resource_request.h"

#include "ipc_util.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
bool ResourceRequest::Marshalling(Parcel& out) const
{
    WRITE_PARCEL_WITH_RET(out, Uint32, allowType_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, uid_, false);
    WRITE_PARCEL_WITH_RET(out, String, name_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, duration_, false);
    WRITE_PARCEL_WITH_RET(out, String, reason_, false);
    WRITE_PARCEL_WITH_RET(out, Uint32, reasonCode_, false);
    return true;
}

ResourceRequest* ResourceRequest::Unmarshalling(Parcel& in)
{
    auto info = new (std::nothrow) ResourceRequest();
    if (info != nullptr && !info->ReadFromParcel(in)) {
        STANDBYSERVICE_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ResourceRequest::ReadFromParcel(Parcel& in)
{
    READ_PARCEL_WITH_RET(in, Uint32, allowType_, false);
    READ_PARCEL_WITH_RET(in, Int32, uid_, false);
    READ_PARCEL_WITH_RET(in, String, name_, false);
    READ_PARCEL_WITH_RET(in, Int32, duration_, false);
    READ_PARCEL_WITH_RET(in, String, reason_, false);
    READ_PARCEL_WITH_RET(in, Uint32, reasonCode_, false);
    return true;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
