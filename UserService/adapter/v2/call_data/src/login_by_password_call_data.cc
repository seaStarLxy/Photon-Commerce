// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "adapter/v2/call_data/include/login_by_password_call_data.h"
#include "adapter/v2/call_data_manager/include/login_by_password_call_data_manager.h"
#include "service/interface/i_auth_service.h"

using namespace user_service::adapter::v2;

LoginByPasswordCallData::LoginByPasswordCallData(LoginByPasswordCallDataManager* manager): CallData(manager) {

}

LoginByPasswordCallData::~LoginByPasswordCallData() = default;

boost::asio::awaitable<void> LoginByPasswordCallData::RunSpecificLogic() {
    auto* auth_service = manager_->GetBusinessService();

    service::LoginByPasswordRequest req;
    req.user_id = request_.user_id();
    req.password = request_.password();
    service::LoginResult result = co_await auth_service->LoginByPassword(req);

    auto* status = reply_.mutable_status();
    status->set_code(static_cast<int32_t>(result.status.code));
    status->set_message(result.status.message);
    if (result.status.code == service::ErrorCode::SUCCESS) {
        reply_.set_token(result.token);
    }
    co_return;
}