// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/auth_service.h"

using namespace user_service::service;

AuthService::AuthService() {

}

AuthService::~AuthService() = default;

boost::asio::awaitable<SendCodeResponse> AuthService::LoginByCode(const SendCodeRequest&) {

}

boost::asio::awaitable<LoginResult> AuthService::LoginByCode(const LoginByCodeRequest&) {

}

boost::asio::awaitable<LoginResult> AuthService::LoginByPassword(const LoginByCodeRequest&) {

}