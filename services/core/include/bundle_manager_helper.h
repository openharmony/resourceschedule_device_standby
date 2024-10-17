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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BUNDLE_MANAGER_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BUNDLE_MANAGER_HELPER_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include "bundle_mgr_interface.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace DevStandbyMgr {
struct UserSpace {
    enum : int {
        INVALID_USERID = -1,
        DEFAULT_USERID = 100,
        SPECIAL_USERID
    };
};
class BundleManagerHelper {
DECLARE_DELAYED_SINGLETON(BundleManagerHelper);
public:
    static std::shared_ptr<BundleManagerHelper> GetInstance();
    std::string GetClientBundleName(int32_t uid);
    bool GetApplicationInfo(const std::string &appName, const AppExecFwk::ApplicationFlag flag,
        const int userId, AppExecFwk::ApplicationInfo &appInfo);
    bool GetApplicationInfos(const AppExecFwk::ApplicationFlag flag, int userId,
        std::vector<AppExecFwk::ApplicationInfo> &appInfos);
    bool CheckIsSystemAppByUid(const int uid, bool& isSystemApp);
private:
    BundleManagerHelper(const BundleManagerHelper&) = delete;
    BundleManagerHelper& operator= (const BundleManagerHelper&) = delete;
    BundleManagerHelper(BundleManagerHelper&&) = delete;
    BundleManagerHelper& operator= (BundleManagerHelper&&) = delete;
    bool Connect();

private:
    sptr<AppExecFwk::IBundleMgr> bundleMgr_ = nullptr;
    std::mutex connectionMutex_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_BUNDLE_MANAGER_HELPER_H