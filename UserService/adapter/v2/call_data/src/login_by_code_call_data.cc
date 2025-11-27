// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "adapter/v2/call_data/include/login_by_code_call_data.h"
#include "adapter/v2/call_data_manager/include/login_by_code_call_data_manager.h"
#include "service/interface/i_auth_service.h"

using namespace user_service::adapter::v2;

LoginByCodeCallData::LoginByCodeCallData(LoginByCodeCallDataManager* manager): CallData(manager) {

}

LoginByCodeCallData::~LoginByCodeCallData() = default;

boost::asio::awaitable<void> LoginByCodeCallData::RunSpecificLogic() {
    auto* auth_service = manager_->GetBusinessService();

    service::LoginByCodeRequest req;
    req.phone_number = request_.phone_number();
    req.code = request_.code();

    service::LoginResult result = co_await auth_service->LoginByCode(req);

    auto* status = reply_.mutable_status();
    status->set_code(static_cast<int32_t>(result.status.code));
    status->set_message(result.status.message);

    if (result.status.code == service::ErrorCode::SUCCESS) {
        reply_.set_token(result.token);
    }
    co_return;
}