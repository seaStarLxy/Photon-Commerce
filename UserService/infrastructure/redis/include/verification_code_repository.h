// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "domain/interface/i_verification_code_repository.h"

namespace user_service::infrastructure {
    class VerificationCodeRepository final: public domain::IVerificationCodeRepository {
    public:
        VerificationCodeRepository();
        ~VerificationCodeRepository() override;
        boost::asio::awaitable<void> SaveCode(const std::string& key, const std::string& code,
            std::chrono::seconds expiry) override;
        boost::asio::awaitable<std::optional<std::string>> GetCode(const std::string& key) override;
    };
}