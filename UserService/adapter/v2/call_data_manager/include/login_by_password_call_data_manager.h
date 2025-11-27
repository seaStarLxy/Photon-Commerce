// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "adapter/v2/call_data_manager/interface/call_data_manager.hpp"
#include "service/interface/i_auth_service.h"

namespace user_service::adapter::v2 {
    class LoginByPasswordCallData;

    class LoginByPasswordCallDataManager final: public CallDataManager<proto::v1::AuthService::AsyncService, LoginByPasswordCallData, service::IAuthService, LoginByPasswordCallDataManager> {
        friend LoginByPasswordCallData;
    public:
        LoginByPasswordCallDataManager(size_t initial_size, proto::v1::AuthService::AsyncService* grpc_service,
            service::IAuthService* business_service, const std::shared_ptr<boost::asio::io_context>& ioc,
            grpc::ServerCompletionQueue *cq);
        ~LoginByPasswordCallDataManager() override;

        void SpecificRegisterCallDataToCQ(LoginByPasswordCallData* call_data) const;
    };
}