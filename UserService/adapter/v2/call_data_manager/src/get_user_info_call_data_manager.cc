// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "adapter/v2/call_data_manager/include/get_user_info_call_data_manager.h"
#include "adapter/v2/call_data/include/get_user_info_call_data.h"

using namespace user_service::adapter::v2;

GetUserInfoCallDataManager::GetUserInfoCallDataManager(const size_t initial_size, proto::v1::UserService::AsyncService* grpc_service,
            service::IBasicUserService* business_service, const std::shared_ptr<boost::asio::io_context>& ioc,
            grpc::ServerCompletionQueue *cq)
            : CallDataManager(initial_size, grpc_service, business_service, ioc, cq) {}

GetUserInfoCallDataManager::~GetUserInfoCallDataManager() = default;

void GetUserInfoCallDataManager::SpecificRegisterCallDataToCQ(GetUserInfoCallData* call_data) const {
    grpc_service_->RequestGetUserInfo(call_data->GetContextAddress(), call_data->GetRequestAddress(), call_data->GetResponderAddress(), cq_, cq_, call_data);
}