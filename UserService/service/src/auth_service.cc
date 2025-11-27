// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "../include/auth_service.h"
#include "service/model/common_model.h"
#include <spdlog/spdlog.h>


using namespace user_service::service;

AuthService::AuthService(const std::shared_ptr<util::IVerificationCodeGenerator>& code_generator,
                         const std::shared_ptr<domain::IVerificationCodeRepository>& code_repository,
                         const std::shared_ptr<domain::IUserRepository>& user_repository,
                         const std::shared_ptr<util::IJwtUtil>& jwt_util,
                         const std::shared_ptr<util::ISecurityUtil>& security_util):
    verification_code_generator_(code_generator), verification_code_repository_(code_repository),
    user_repository_(user_repository), jwt_util_(jwt_util), security_util_(security_util) {
    SPDLOG_INFO("AuthService Init: JwtUtil Address: {}", fmt::ptr(jwt_util_.get()));

    if (!jwt_util_) {
        SPDLOG_CRITICAL("AuthService CRITICAL ERROR: jwt_util is nullptr! Injection Failed.");
        throw std::runtime_error("Injection Failed: jwt_util is null");
    }
    SPDLOG_DEBUG("AuthService Created");
}

AuthService::~AuthService() = default;

boost::asio::awaitable<SendCodeResponse> AuthService::SendCode(const SendCodeRequest& req) {
    // 标记是否需要真正执行发送逻辑
    bool should_send_code = true;

    switch (req.usage)
    {
        case USER_REGISTER:
            // 注册场景下不做任何检查，直接发
            should_send_code = true;
            break;
        case USER_LOGIN:
        case RESET_PASSWORD:
        {
            auto user_result_exp = co_await user_repository_->GetUserByPhoneNumber(req.phone_number);
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
                SPDLOG_INFO("Security: Suppressed SendCode request for non-existent user: {}", req.phone_number);
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
    SPDLOG_DEBUG("send code {} to {}", code, req.phone_number);
    // 存到 redis
    const auto expiry = std::chrono::minutes(5); // TTL: 5分钟
    co_await verification_code_repository_->SaveCode(req.usage,
                                                     req.phone_number, code, expiry);
    SPDLOG_DEBUG("Verification code saved to Redis successfully");

    co_return SendCodeResponse(CommonStatus::Success());
}

boost::asio::awaitable<LoginResult> AuthService::LoginByCode(const LoginByCodeRequest& req) {
    // 校验验证码
    const auto code_opt = co_await verification_code_repository_->GetCode(CodeUsage::USER_LOGIN, req.phone_number);
    if (!code_opt.has_value() || code_opt.value() != req.code) {
        co_return LoginResult{CommonStatus(ErrorCode::VERIFICATION_CODE_MISMATCH, "验证码错误")};
    }

    // 按手机号查用户
    const auto user_exp = co_await user_repository_->GetUserByPhoneNumber(req.phone_number);
    if (!user_exp.has_value()) co_return LoginResult{CommonStatus(ErrorCode::INTERNAL_ERROR, "DB Error")};

    const auto user_opt = user_exp.value();
    if (!user_opt.has_value()) {
        co_return LoginResult{CommonStatus(ErrorCode::USER_NOT_FOUND, "该手机号未注册")};
    }
    const auto& user = user_opt.value();

    // 签发 Token
    const std::string token = jwt_util_->GenerateToken(user.GetId());

    co_return LoginResult{CommonStatus::Success(), token, user.GetId()};
}

boost::asio::awaitable<LoginResult> AuthService::LoginByPassword(const LoginByPasswordRequest& req) {
    SPDLOG_DEBUG("{}: {}", req.user_id, req.password);
    // 通过 user_id 查询
    const auto user_exp = co_await user_repository_->GetUserById(req.user_id);

    if (!user_exp.has_value()) co_return LoginResult{CommonStatus(ErrorCode::INTERNAL_ERROR, "DB Error")};

    const auto user_opt = user_exp.value();
    if (!user_opt.has_value()) {
        co_return LoginResult{CommonStatus(ErrorCode::USER_NOT_FOUND, "ID不存在")};
    }
    const auto& user = user_opt.value();

    // 验证密码
    if (!security_util_->VerifyPassword(req.password, user.GetSalt(), user.GetPasswordHash())) {
        co_return LoginResult{CommonStatus(ErrorCode::PASSWORD_INCORRECT, "密码错误")};
    }

    // 签发 Token
    const std::string token = jwt_util_->GenerateToken(user.GetId());

    co_return LoginResult{CommonStatus::Success(), token, user.GetId()};
}
