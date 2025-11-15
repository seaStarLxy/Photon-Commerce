// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/auth_service.h"
#include <spdlog/spdlog.h>

using namespace user_service::service;

AuthService::AuthService(const std::shared_ptr<util::ICodeGenerator>& code_generator,
    const std::shared_ptr<domain::IVerificationCodeRepository>& code_repository):
    code_generator_(code_generator), code_repository_(code_repository) {

}

AuthService::~AuthService() = default;

boost::asio::awaitable<SendCodeResponse> AuthService::SendCode(const SendCodeRequest& send_code_request) {
    // 生成验证码
    std::string code = code_generator_->Generate(6);
    // 发送验证码
    SPDLOG_DEBUG("send code {} to {}", code, send_code_request.phone_number);
    // 存到 redis
    std::string redis_key = std::to_string(send_code_request.usage) + ":" + send_code_request.phone_number;
    auto expiry = std::chrono::minutes(5);
    co_await code_repository_->SaveCode(redis_key, code, expiry);
    SPDLOG_DEBUG("saved to redis");
    co_return SendCodeResponse(true);
}

boost::asio::awaitable<LoginResult> AuthService::LoginByCode(const LoginByCodeRequest&) {

}

boost::asio::awaitable<LoginResult> AuthService::LoginByPassword(const LoginByCodeRequest&) {

}