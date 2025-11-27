// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "adapter/v2/call_data/interface/call_data.hpp"
#include <UserService/v1/user_service.grpc.pb.h>

namespace user_service::adapter::v2 {
    class LoginByPasswordCallDataManager;

    class LoginByPasswordCallData final: public CallData<proto::v1::LoginByPasswordRequest, proto::v1::LoginResponse, LoginByPasswordCallDataManager, LoginByPasswordCallData> {
        friend LoginByPasswordCallDataManager;
    public:
        explicit LoginByPasswordCallData(LoginByPasswordCallDataManager* manager);
        ~LoginByPasswordCallData() override;
        boost::asio::awaitable<void> RunSpecificLogic();
    };
}