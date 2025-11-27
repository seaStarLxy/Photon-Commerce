// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "adapter/v2/call_data_manager/interface/call_data_manager.hpp"
#include "service/interface/i_basic_user_service.h"

namespace user_service::adapter::v2 {
    class GetUserInfoCallData;

    class GetUserInfoCallDataManager final: public CallDataManager<proto::v1::UserService::AsyncService, GetUserInfoCallData, service::IBasicUserService, GetUserInfoCallDataManager> {
        friend GetUserInfoCallData;
    public:
        GetUserInfoCallDataManager(size_t initial_size, proto::v1::UserService::AsyncService* grpc_service,
            service::IBasicUserService* business_service, const std::shared_ptr<boost::asio::io_context>& ioc,
            grpc::ServerCompletionQueue *cq);
        ~GetUserInfoCallDataManager() override;

        void SpecificRegisterCallDataToCQ(GetUserInfoCallData* call_data) const;
    };
}