// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "service/interface/i_basic_user_service.h"

namespace user_service::service {
    class BasicUserService final: public IBasicUserService {
    public:
        BasicUserService();
        ~BasicUserService() override;
        boost::asio::awaitable<RegisterResponse> Register(const RegisterRequest&) override;
        boost::asio::awaitable<GetUserInfoResponse> GetUserInfo(const GetUserInfoRequest&) override;
        boost::asio::awaitable<UpdateUserInfoResponse> UpdateUserInfo(const UpdateUserInfoRequest&) override;
    };
}
