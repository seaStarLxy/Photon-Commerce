// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "../include/basic_user_service.h"
#include <spdlog/spdlog.h>

using namespace user_service::service;
using namespace user_service::domain;

BasicUserService::BasicUserService(const std::shared_ptr<domain::IUserRepository>& user_repo,
            const std::shared_ptr<domain::IVerificationCodeRepository>& code_repo,
            const std::shared_ptr<util::IIDGenerator>& id_gen,
            const std::shared_ptr<util::ISecurityUtil>& security_util): user_repository_(user_repo),
    code_repository_(code_repo), id_generator_(id_gen), security_util_(security_util) {
    SPDLOG_DEBUG("BasicUserService Created");
}

BasicUserService::~BasicUserService() = default;

boost::asio::awaitable<RegisterResponse> BasicUserService::Register(const RegisterRequest& req) {
    // 校验验证码
    const auto code_opt = co_await code_repository_->GetCode(CodeUsage::USER_REGISTER, req.phone_number);
    if (!code_opt.has_value() || code_opt.value() != req.code) {
        co_return RegisterResponse{CommonStatus(ErrorCode::VERIFICATION_CODE_MISMATCH, "验证码错误")};
    }

    // 检查手机号是否存在
    const auto existing_user = co_await user_repository_->GetUserByPhoneNumber(req.phone_number);
    if (!existing_user.has_value()) {
        co_return RegisterResponse{CommonStatus(ErrorCode::INTERNAL_ERROR, "Database Error")};
    }
    if (existing_user.value().has_value()) {
        co_return RegisterResponse{CommonStatus(ErrorCode::USER_ALREADY_EXISTS, "该手机号已注册")};
    }

    // 生成 ID 和 密码哈希
    const std::string user_id = id_generator_->GenerateUUID();
    const std::string salt = security_util_->GenerateSalt();
    const std::string pwd_hash = security_util_->HashPassword(req.password, salt);

    // 构建对象
    User new_user = User::Create(user_id, req.phone_number, pwd_hash, salt);
    // 设置额外的用户名信息
    new_user.UpdateProfile(req.username, std::nullopt, std::nullopt);

    // 写入数据库
    const auto result = co_await user_repository_->CreateUser(new_user);
    if (!result.has_value()) {
        SPDLOG_ERROR("CreateUser failed: {}", result.error().pg_error_message);
        co_return RegisterResponse{CommonStatus(ErrorCode::INTERNAL_ERROR, "注册失败")};
    }

    co_return RegisterResponse{CommonStatus::Success(), user_id};
}

boost::asio::awaitable<GetUserInfoResponse> BasicUserService::GetUserInfo(const GetUserInfoRequest& req) {
    // 根据 user_id 查询
    const auto user_exp = co_await user_repository_->GetUserById(req.user_id);

    if (!user_exp.has_value()) {
        co_return GetUserInfoResponse{CommonStatus(ErrorCode::INTERNAL_ERROR, "查询失败")};
    }

    const auto user_opt = user_exp.value();
    if (!user_opt.has_value()) {
        co_return GetUserInfoResponse{CommonStatus(ErrorCode::USER_NOT_FOUND, "用户不存在")};
    }

    const auto& user = user_opt.value();
    co_return GetUserInfoResponse{
        CommonStatus::Success(),
        user.GetId(),
        user.GetUsername().value_or(""),
        user.GetEmail().value_or(""),
        user.GetAvatarUrl().value_or("")
    };
}

boost::asio::awaitable<UpdateUserInfoResponse> BasicUserService::UpdateUserInfo(const UpdateUserInfoRequest&) {
    co_return UpdateUserInfoResponse{CommonStatus(ErrorCode::INTERNAL_ERROR, "Not Implemented Yet")};
}