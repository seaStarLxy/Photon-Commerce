// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/auth_service.h"
#include "service/model/common_model.h"
#include <spdlog/spdlog.h>


using namespace user_service::service;

AuthService::AuthService(const std::shared_ptr<util::IVerificationCodeGenerator>& code_generator,
                         const std::shared_ptr<domain::IVerificationCodeRepository>& code_repository,
                         const std::shared_ptr<domain::IUserRepository>& user_repository):
    verification_code_generator_(code_generator), verification_code_repository_(code_repository), user_repository_(user_repository)
{
    SPDLOG_DEBUG("Execute AuthService Constructor");
}

AuthService::~AuthService() = default;

boost::asio::awaitable<SendCodeResponse> AuthService::SendCode(const SendCodeRequest& send_code_request)
{
    // 1. 查询用户是否存在
    bool user_exists = false;
    // 2. 根据验证码用途进行逻辑校验
    switch (send_code_request.usage)
    {
    case USER_REGISTER:
        if (user_exists)
        {
            SPDLOG_WARN("Registration failed: User {} already exists", send_code_request.phone_number);
            co_return SendCodeResponse(CommonStatus(ErrorCode::USER_NOT_FOUND, "该手机号已注册"));
        }
        break;
    case USER_LOGIN:
        if (!user_exists)
        {
            co_return SendCodeResponse(CommonStatus(ErrorCode::USER_NOT_FOUND, "该手机号尚未注册"));
        }
        break;
    case RESET_PASSWORD:
        if (!user_exists)
        {
            co_return SendCodeResponse(CommonStatus(ErrorCode::USER_NOT_FOUND, "该手机号尚未注册"));
        }
        break;
    default:
        co_return SendCodeResponse(CommonStatus(ErrorCode::INVALID_ARGUMENT, "发送验证码用途不明"));
    }
    // 3. 生成验证码
    std::string code = verification_code_generator_->Generate(6);
    // 4. 模拟发送验证码
    SPDLOG_DEBUG("send code {} to {}", code, send_code_request.phone_number);
    // 5. 存到 redis
    const auto expiry = std::chrono::minutes(5); // TTL: 5分钟
    co_await verification_code_repository_->SaveCode(send_code_request.usage,
                                                     send_code_request.phone_number, code, expiry);
    SPDLOG_DEBUG("Verification code saved to Redis successfully");

    co_return SendCodeResponse(CommonStatus::Success());
}

boost::asio::awaitable<LoginResult> AuthService::LoginByCode(const LoginByCodeRequest& login_by_code_request)
{
}

boost::asio::awaitable<LoginResult> AuthService::LoginByPassword(
    const LoginByPasswordRequest& login_by_password_request)
{
}
