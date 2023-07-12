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

#include "init.h"

#include "allow_type.h"
#include "common.h"
#include "standby_napi_module.h"

namespace OHOS {
namespace DevStandbyMgr {
EXTERN_C_START

napi_value DeviceStandbyFuncInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("isDeviceInStandby", IsDeviceInStandby),
        DECLARE_NAPI_FUNCTION("getExemptedApps", GetExemptionListApps),
        DECLARE_NAPI_FUNCTION("requestExemptionResource", ApplyAllowResource),
        DECLARE_NAPI_FUNCTION("releaseExemptionResource", UnapplyAllowResource),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_int32(env, objName, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value DeviceStandbyTypeInit(napi_env env, napi_value exports)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(AllowType::NETWORK), "NETWORK");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(AllowType::RUNNING_LOCK), "RUNNING_LOCK");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(AllowType::TIMER), "TIMER");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(AllowType::WORK_SCHEDULER), "WORK_SCHEDULER");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(AllowType::AUTO_SYNC), "AUTO_SYNC");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(AllowType::PUSH), "PUSH");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(AllowType::FREEZE), "FREEZE");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("ResourceType", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

/*
 * Module export function
 */
static napi_value DeviceStandbyInit(napi_env env, napi_value exports)
{
    /*
     * Properties define
     */
    DeviceStandbyFuncInit(env, exports);
    DeviceStandbyTypeInit(env, exports);
    return exports;
}

/*
 * Module register function
 */
__attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
EXTERN_C_END
}  // namespace DevStandbyMgr
}  // namespace OHOS
