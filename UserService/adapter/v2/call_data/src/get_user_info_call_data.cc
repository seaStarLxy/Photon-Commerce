// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "adapter/v2/call_data/include/get_user_info_call_data.h"
#include "adapter/v2/call_data_manager/include/get_user_info_call_data_manager.h"
#include "service/interface/i_basic_user_service.h"

using namespace user_service::adapter::v2;

GetUserInfoCallData::GetUserInfoCallData(GetUserInfoCallDataManager* manager): CallData(manager) {

}

GetUserInfoCallData::~GetUserInfoCallData() = default;

boost::asio::awaitable<void> GetUserInfoCallData::RunSpecificLogic() {
    auto* basic_service = manager_->GetBusinessService();

    service::GetUserInfoRequest req;
    req.user_id = request_.user_id();

    service::GetUserInfoResponse result = co_await basic_service->GetUserInfo(req);

    auto* status = reply_.mutable_status();
    status->set_code(static_cast<int32_t>(result.status.code));
    status->set_message(result.status.message);

    if (result.status.code == service::ErrorCode::SUCCESS) {
        auto* user = reply_.mutable_user();
        user->set_user_id(result.user_id);
        user->set_username(result.username);
        user->set_email(result.email);
        user->set_avatar_url(result.avatar_url);
        // Phone number 涉及隐私，视业务需求决定是否返回
        user->set_phone_number("");
    }
    co_return;
}