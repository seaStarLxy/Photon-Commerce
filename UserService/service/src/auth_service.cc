// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/auth_service.h"
#include <spdlog/spdlog.h>

using namespace user_service::service;

AuthService::AuthService(const std::shared_ptr<util::IVerificationCodeGenerator>& code_generator,
    const std::shared_ptr<domain::IVerificationCodeRepository>& code_repository):
    verification_code_generator_(code_generator), verification_code_repository_(code_repository) {
    SPDLOG_DEBUG("Execute AuthService Constructor");
}

AuthService::~AuthService() = default;

boost::asio::awaitable<SendCodeResponse> AuthService::SendCode(const SendCodeRequest& send_code_request) {
    // 生成验证码
    std::string code = verification_code_generator_->Generate(6);
    // 模拟发送验证码
    SPDLOG_DEBUG("send code {} to {}", code, send_code_request.phone_number);
    // 存到 redis
    const auto expiry = std::chrono::minutes(5);
    co_await verification_code_repository_->SaveCode(send_code_request.usage,
        send_code_request.phone_number, code, expiry);
    SPDLOG_DEBUG("saved to redis");
    // test
    const auto back_code = co_await verification_code_repository_->GetCode(send_code_request.usage, send_code_request.phone_number);
    if (back_code)
    {
        SPDLOG_DEBUG("result code: {}", back_code.value());
    }
    else
    {
        SPDLOG_DEBUG("no result");
    }
    co_return SendCodeResponse(true);
}

boost::asio::awaitable<LoginResult> AuthService::LoginByCode(const LoginByCodeRequest&) {

}

boost::asio::awaitable<LoginResult> AuthService::LoginByPassword(const LoginByCodeRequest&) {

}