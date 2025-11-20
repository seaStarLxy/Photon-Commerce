// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "adapter/v2/call_data_manager/include/register_call_data_manager.h"
#include "adapter/v2/call_data/include/register_call_data.h"

using namespace user_service::adapter::v2;

RegisterCallDataManager::RegisterCallDataManager(const size_t initial_size, proto::v1::UserService::AsyncService* grpc_service,
            service::IBasicUserService* business_service, const std::shared_ptr<boost::asio::io_context>& ioc,
            grpc::ServerCompletionQueue *cq): CallDataManager(initial_size, grpc_service, business_service, ioc, cq) {
    SPDLOG_INFO("DEBUG CHECK: RegisterCallDataManager ioc address: {}", fmt::ptr(ioc_.get()));
}

RegisterCallDataManager::~RegisterCallDataManager() = default;

void RegisterCallDataManager::SpecificRegisterCallDataToCQ(RegisterCallData* call_data) const {

    // grpc_service_->RequestRegister(&call_data->ctx_, &call_data->request_, &call_data->responder_, cq_, cq_, call_data);
    grpc_service_->RequestRegister(call_data->GetContextAddress(), call_data->GetRequestAddress(), call_data->GetResponderAddress(), cq_, cq_, call_data);

}