// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "domain/interface/i_verification_code_repository.h"
#include "infrastructure/state_storage/redis/redis_client.h"
#include "service/model/auth_model.h"

namespace user_service::infrastructure
{
    class VerificationCodeRepository final : public domain::IVerificationCodeRepository
    {
    public:
        explicit VerificationCodeRepository(const std::shared_ptr<RedisClient>& redis_client);
        ~VerificationCodeRepository() override;

        boost::asio::awaitable<void> SaveCode(const service::CodeUsage& usage, const std::string& target,
                                              const std::string& code, std::chrono::seconds expiry) override;

        boost::asio::awaitable<std::optional<std::string>> GetCode(const service::CodeUsage& usage, const std::string& target) override;

    private:
        std::string GetKeyPrefixForUsage(const service::CodeUsage& usage) const;
        const std::shared_ptr<RedisClient> redis_client_;
    };
}
