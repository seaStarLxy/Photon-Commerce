// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "utils/interface/i_jwt_util.h"

namespace user_service::util {
    struct JwtConfig {
        std::string secret_key;
        std::string issuer;
        int expiration_seconds;
    };

    class JwtUtil: public IJwtUtil {
        static constexpr auto CLAIM_USER_ID = "user_id";
        static constexpr auto TOKEN_TYPE = "JWS";
    public:
        explicit JwtUtil(const JwtConfig& config);
        ~JwtUtil() override;

        std::string GenerateToken(const std::string& user_id) override;
        std::expected<std::string, JwtError> VerifyToken(const std::string& token) override;

    private:
        const std::string secret_key_;
        const std::string issuer_;
        const std::chrono::seconds expiration_seconds_;
    };
}