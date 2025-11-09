// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "../model/basic_user_model.h"
#include <boost/asio.hpp>

namespace user_service::service {
    class IBasicUserService {
    public:
        virtual ~IBasicUserService() = default;
        virtual boost::asio::awaitable<RegisterResponse> Register(const RegisterRequest&) = 0;
        virtual boost::asio::awaitable<GetUserInfoResponse> GetUserInfo(const GetUserInfoRequest&) = 0;
        virtual boost::asio::awaitable<UpdateUserInfoResponse> UpdateUserInfo(const UpdateUserInfoRequest&) = 0;
    };
}