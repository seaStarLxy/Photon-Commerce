// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/basic_user_service.h"

using namespace user_service::service;

BasicUserService::BasicUserService() {

}

BasicUserService::~BasicUserService() = default;

boost::asio::awaitable<RegisterResponse> BasicUserService::Register(const RegisterRequest& register_request) {
    std::string prefix("Hello,  ");
    RegisterResponse register_response(prefix+register_request.username, register_request.code);
    co_return register_response;
}

boost::asio::awaitable<GetUserInfoResponse> BasicUserService::GetUserInfo(const GetUserInfoRequest&) {

}

boost::asio::awaitable<UpdateUserInfoResponse> BasicUserService::UpdateUserInfo(const UpdateUserInfoRequest&) {

}