// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <boost/asio.hpp>

namespace user_service::domain {
    class IVerificationCodeRepository {
    public:
        virtual ~IVerificationCodeRepository() = default;

        virtual boost::asio::awaitable<void> SaveCode(const std::string& key, const std::string& code,
            std::chrono::seconds expiry) = 0;

        virtual boost::asio::awaitable<std::optional<std::string>> GetCode(const std::string& key) = 0;
    };
}