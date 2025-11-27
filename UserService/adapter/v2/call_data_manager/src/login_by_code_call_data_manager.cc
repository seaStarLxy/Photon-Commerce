// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "adapter/v2/call_data_manager/include/login_by_code_call_data_manager.h"
#include "adapter/v2/call_data/include/login_by_code_call_data.h"

using namespace user_service::adapter::v2;

LoginByCodeCallDataManager::LoginByCodeCallDataManager(const size_t initial_size, proto::v1::AuthService::AsyncService* grpc_service,
            service::IAuthService* business_service, const std::shared_ptr<boost::asio::io_context>& ioc,
            grpc::ServerCompletionQueue *cq)
            : CallDataManager(initial_size, grpc_service, business_service, ioc, cq) {}

LoginByCodeCallDataManager::~LoginByCodeCallDataManager() = default;

void LoginByCodeCallDataManager::SpecificRegisterCallDataToCQ(LoginByCodeCallData* call_data) const {
    grpc_service_->RequestLoginByCode(call_data->GetContextAddress(), call_data->GetRequestAddress(), call_data->GetResponderAddress(), cq_, cq_, call_data);
}