// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "adapter/v2/call_data/interface/call_data.hpp"
#include <UserService/v1/user_service.grpc.pb.h>

namespace user_service::adapter::v2 {
    class LoginByCodeCallDataManager;

    class LoginByCodeCallData final: public CallData<proto::v1::LoginByCodeRequest, proto::v1::LoginResponse, LoginByCodeCallDataManager, LoginByCodeCallData> {
        friend LoginByCodeCallDataManager;
    public:
        explicit LoginByCodeCallData(LoginByCodeCallDataManager* manager);
        ~LoginByCodeCallData() override;
        boost::asio::awaitable<void> RunSpecificLogic();
    };
}