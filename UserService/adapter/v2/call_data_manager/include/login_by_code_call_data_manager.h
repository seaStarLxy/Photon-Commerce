// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "adapter/v2/call_data_manager/interface/call_data_manager.hpp"
#include "service/interface/i_auth_service.h"

namespace user_service::adapter::v2 {
    class LoginByCodeCallData;

    class LoginByCodeCallDataManager final: public CallDataManager<proto::v1::AuthService::AsyncService, LoginByCodeCallData, service::IAuthService, LoginByCodeCallDataManager> {
        friend LoginByCodeCallData;
    public:
        LoginByCodeCallDataManager(size_t initial_size, proto::v1::AuthService::AsyncService* grpc_service,
            service::IAuthService* business_service, const std::shared_ptr<boost::asio::io_context>& ioc,
            grpc::ServerCompletionQueue *cq);
        ~LoginByCodeCallDataManager() override;

        void SpecificRegisterCallDataToCQ(LoginByCodeCallData* call_data) const;
    };
}