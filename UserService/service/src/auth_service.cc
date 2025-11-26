// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

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
    // 标记是否需要真正执行发送逻辑
    bool should_send_code = true;

    switch (send_code_request.usage)
    {
        case USER_REGISTER:
            // 注册场景下不做任何检查，直接发
            should_send_code = true;
            break;
        case USER_LOGIN:
        case RESET_PASSWORD:
        {
            auto user_result_exp = co_await user_repository_->GetUserByPhoneNumber(send_code_request.phone_number);
            // 系统级错误
            if (!user_result_exp.has_value()) {
                const auto& db_err = user_result_exp.error();
                SPDLOG_ERROR("DB Error in SendCode: {}", db_err.pg_error_message);
                co_return SendCodeResponse(CommonStatus(ErrorCode::INTERNAL_ERROR, "系统内部错误"));
            }
            // 业务级检查
            const auto& user_opt = user_result_exp.value();
            if (!user_opt.has_value()) {
                // 用户不存在则沉默处理，防止枚举攻击
                SPDLOG_INFO("Security: Suppressed SendCode request for non-existent user: {}", send_code_request.phone_number);
                should_send_code = false;
            } else {
                should_send_code = true;
            }
            break;
        }
        default:
            co_return SendCodeResponse(CommonStatus(ErrorCode::INVALID_ARGUMENT, "发送验证码用途不明"));
    }

    // 判断是否需要真发
    if (!should_send_code) {
        co_return SendCodeResponse(CommonStatus::Success());
    }
    // 生成验证码
    std::string code = verification_code_generator_->Generate(6);
    // 模拟发送验证码
    SPDLOG_DEBUG("send code {} to {}", code, send_code_request.phone_number);
    // 存到 redis
    const auto expiry = std::chrono::minutes(5); // TTL: 5分钟
    co_await verification_code_repository_->SaveCode(send_code_request.usage,
                                                     send_code_request.phone_number, code, expiry);
    SPDLOG_DEBUG("Verification code saved to Redis successfully");

    co_return SendCodeResponse(CommonStatus::Success());
}

boost::asio::awaitable<LoginResult> AuthService::LoginByCode(const LoginByCodeRequest& login_by_code_request) {
    co_return LoginResult{CommonStatus(ErrorCode::INTERNAL_ERROR, "Not Implemented Yet")};
}

boost::asio::awaitable<LoginResult> AuthService::LoginByPassword(
    const LoginByPasswordRequest& login_by_password_request) {
    co_return LoginResult{CommonStatus(ErrorCode::INTERNAL_ERROR, "Not Implemented Yet")};
}
