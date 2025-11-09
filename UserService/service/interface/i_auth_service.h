// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "../model/auth_model.h"
#include <boost/asio.hpp>

namespace user_service::service {

    class IAuthService {
    public:
        virtual ~IAuthService() = default;
        virtual boost::asio::awaitable<SendCodeResponse> LoginByCode(const SendCodeRequest&) = 0;
        virtual boost::asio::awaitable<LoginResult> LoginByCode(const LoginByCodeRequest&) = 0;
        virtual boost::asio::awaitable<LoginResult> LoginByPassword(const LoginByCodeRequest&) = 0;
    };
}