// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/verification_code_repository.h"

#include "spdlog/spdlog.h"

using namespace user_service::infrastructure;
using namespace user_service::service;

VerificationCodeRepository::VerificationCodeRepository(
    const std::shared_ptr<RedisClient>& redis_client) : redis_client_(redis_client)
{
    SPDLOG_DEBUG("Execute VerificationCodeRepository Constructor");
}

VerificationCodeRepository::~VerificationCodeRepository() = default;

boost::asio::awaitable<void> VerificationCodeRepository::SaveCode(const CodeUsage& usage, const std::string& target,
                                                                  const std::string& code, std::chrono::seconds expiry)
{
    std::string redis_key;
    try {
        // 构建 key
        redis_key = GetKeyPrefixForUsage(usage) + target;
    } catch (const std::invalid_argument& e) {
        SPDLOG_ERROR("Failed to save code due to invalid usage type: {}", e.what());
        co_return;
    }
    SPDLOG_DEBUG("Saving code to redis. Key: [{}], Code: [{}], Expiry: [{}s]",
                 redis_key, code, expiry.count());
    co_await redis_client_->Set(redis_key, code, expiry);
}

boost::asio::awaitable<std::optional<std::string>> VerificationCodeRepository::GetCode(const CodeUsage& usage, const std::string& target)
{
    std::string redis_key;
    try {
        // 构建 key
        redis_key = GetKeyPrefixForUsage(usage) + target;
    } catch (const std::invalid_argument& e) {
        SPDLOG_ERROR("Failed to get code due to invalid usage type: {}", e.what());
        co_return std::nullopt;
    }
    SPDLOG_DEBUG("Getting code from redis. Key: [{}]", redis_key);
    co_return co_await redis_client_->Get(redis_key);
}

std::string VerificationCodeRepository::GetKeyPrefixForUsage(const CodeUsage& usage) const
{
    switch (usage)
    {
    case USER_REGISTER:
        return "verify_code:register:";
    case USER_LOGIN:
        return "verify_code:login:";
    case RESET_PASSWORD:
        return "verify_code:reset_pwd:";
    default:
        SPDLOG_ERROR("Unknown CodeUsage enum value: {}", static_cast<int>(usage));
        throw std::invalid_argument("Unknown CodeUsage");
    }
}
