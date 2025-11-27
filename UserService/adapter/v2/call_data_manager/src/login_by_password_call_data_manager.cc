// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "adapter/v2/call_data_manager/include/login_by_password_call_data_manager.h"
#include "adapter/v2/call_data/include/login_by_password_call_data.h"

using namespace user_service::adapter::v2;

LoginByPasswordCallDataManager::LoginByPasswordCallDataManager(const size_t initial_size, proto::v1::AuthService::AsyncService* grpc_service,
            service::IAuthService* business_service, const std::shared_ptr<boost::asio::io_context>& ioc,
            grpc::ServerCompletionQueue *cq)
            : CallDataManager(initial_size, grpc_service, business_service, ioc, cq) {}

LoginByPasswordCallDataManager::~LoginByPasswordCallDataManager() = default;

void LoginByPasswordCallDataManager::SpecificRegisterCallDataToCQ(LoginByPasswordCallData* call_data) const {
    grpc_service_->RequestLoginByPassword(call_data->GetContextAddress(), call_data->GetRequestAddress(), call_data->GetResponderAddress(), cq_, cq_, call_data);
}