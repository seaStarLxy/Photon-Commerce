// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "service/interface/i_auth_service.h"

namespace user_service::service {
    class AuthService final: public IAuthService {
    public:
        AuthService();
        ~AuthService() override;
        boost::asio::awaitable<SendCodeResponse> LoginByCode(const SendCodeRequest&) override;
        boost::asio::awaitable<LoginResult> LoginByCode(const LoginByCodeRequest&) override;
        boost::asio::awaitable<LoginResult> LoginByPassword(const LoginByCodeRequest&) override;
    };
}
