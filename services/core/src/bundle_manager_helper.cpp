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

#include "bundle_manager_helper.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "standby_service_errors.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
IMPLEMENT_SINGLE_INSTANCE(BundleManagerHelper);
BundleManagerHelper::BundleManagerHelper()
{
}

BundleManagerHelper::~BundleManagerHelper()
{
}

std::string WEAK_FUNC BundleManagerHelper::GetClientBundleName(int32_t uid)
{
    std::string bundle {""};
    std::lock_guard<std::mutex> lock(connectionMutex_);
    Connect();
    if (bundleMgr_ != nullptr) {
        bundleMgr_->GetNameForUid(uid, bundle);
    }
    STANDBYSERVICE_LOGD("get client Bundle Name: %{public}s", bundle.c_str());
    return bundle;
}

bool WEAK_FUNC BundleManagerHelper::GetApplicationInfo(const std::string &appName, const
    AppExecFwk::ApplicationFlag flag, const int userId, AppExecFwk::ApplicationInfo &appInfo)
{
    STANDBYSERVICE_LOGD("start get application info");
    std::lock_guard<std::mutex> lock(connectionMutex_);

    Connect();
    STANDBYSERVICE_LOGD("bundleMgr is null: %{public}d ", bundleMgr_ == nullptr);
    if (bundleMgr_ == nullptr || !bundleMgr_->GetApplicationInfo(appName, flag, userId, appInfo)) {
        return false;
    }
    return true;
}

bool WEAK_FUNC BundleManagerHelper::GetApplicationInfos(const AppExecFwk::ApplicationFlag flag, int userId,
    std::vector<AppExecFwk::ApplicationInfo> &appInfos)
{
    std::lock_guard<std::mutex> lock(connectionMutex_);

    Connect();
    STANDBYSERVICE_LOGD("bundleMgr is null: %{public}d ", bundleMgr_ == nullptr);
    if (bundleMgr_ == nullptr || !bundleMgr_->GetApplicationInfos(flag, userId, appInfos)) {
        return false;
    }
    return true;
}

bool WEAK_FUNC BundleManagerHelper::CheckIsSystemAppByUid(const int uid, bool& isSystemApp)
{
    std::lock_guard<std::mutex> lock(connectionMutex_);
    Connect();
    STANDBYSERVICE_LOGD("bundleMgr is null: %{public}d ", bundleMgr_ == nullptr);
    if (bundleMgr_ == nullptr) {
        return false;
    }
    isSystemApp = bundleMgr_->CheckIsSystemAppByUid(uid);
    return true;
}

bool WEAK_FUNC BundleManagerHelper::Connect()
{
    if (bundleMgr_ != nullptr) {
        return true;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        STANDBYSERVICE_LOGE("get SystemAbilityManager failed");
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        STANDBYSERVICE_LOGE("get Bundle Manager failed");
        return false;
    }

    bundleMgr_ = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr_ == nullptr) {
        STANDBYSERVICE_LOGE("get bundleMgr failed");
        return false;
    }
    return true;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS