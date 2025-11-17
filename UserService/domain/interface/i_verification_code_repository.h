// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "service/model/auth_model.h"
#include <boost/asio.hpp>
#include <optional>

namespace user_service::domain {
    class IVerificationCodeRepository {
    public:
        virtual ~IVerificationCodeRepository() = default;

        virtual boost::asio::awaitable<void> SaveCode(const service::CodeUsage& usage, const std::string& target,
            const std::string& code, std::chrono::seconds expiry) = 0;

        virtual boost::asio::awaitable<std::optional<std::string>> GetCode(const service::CodeUsage& usage, const std::string& target) = 0;
    };
}