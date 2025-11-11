// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/register_call_data.h"

using namespace user_service::adapter::v2;


RegisterCallData::RegisterCallData(ICallDataManager* manager): ICallData(manager) {

}
RegisterCallData::~RegisterCallData() = default;

boost::asio::awaitable<void> RegisterCallData::RunLogic() {
    SPDLOG_DEBUG("run RunLogic");
    service::IBasicUserService* basic_user_service = static_cast<service::IBasicUserService*>(manager_->GetBusinessService());
    RegisterRequest register_request(request_.username(), request_.password(),
        request_.phone_number(), request_.code());
    SPDLOG_DEBUG("ready to enter coroutine");
    RegisterResponse register_response = co_await basic_user_service->Register(register_request);
    SPDLOG_DEBUG("leave from coroutine");
    reply_.set_user_id(register_response.user_id);
    reply_.set_token(register_response.token);
    co_return;
}
