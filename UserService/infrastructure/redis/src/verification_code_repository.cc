// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/verification_code_repository.h"

#include "spdlog/spdlog.h"

using namespace user_service::infrastructure;

VerificationCodeRepository::VerificationCodeRepository() {

}

VerificationCodeRepository::~VerificationCodeRepository() = default;

boost::asio::awaitable<void> VerificationCodeRepository::SaveCode(const std::string& key, const std::string& code,
    std::chrono::seconds expiry) {
    SPDLOG_DEBUG("save to redis {}", code);
    co_return;
}

boost::asio::awaitable<std::optional<std::string>> VerificationCodeRepository::GetCode(const std::string& key) {

}