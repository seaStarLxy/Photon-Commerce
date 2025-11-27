// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#undef JWT_DISABLE_PICOJSON
#include "utils/include/jwt_util.h"
#include <jwt-cpp/jwt.h>
#include <chrono>
#include <spdlog/spdlog.h>

using namespace user_service::util;

JwtUtil::JwtUtil(const JwtConfig& config)
    : secret_key_(config.secret_key)
    , issuer_(config.issuer)
    , expiration_seconds_(config.expiration_seconds) {
    SPDLOG_DEBUG("JwtUtil initialized. Issuer: {}, Expiry: {}s", issuer_, expiration_seconds_.count());
}

JwtUtil::~JwtUtil() = default;

std::string JwtUtil::GenerateToken(const std::string& user_id) {
    const auto now = std::chrono::system_clock::now();

    return jwt::create()
        .set_issuer(issuer_)
        .set_type(TOKEN_TYPE)
        .set_payload_claim(CLAIM_USER_ID, picojson::value(user_id))
        .set_issued_at(now)
        .set_expires_at(now + expiration_seconds_)
        .sign(jwt::algorithm::hs256{secret_key_});
}

std::expected<std::string, JwtError> JwtUtil::VerifyToken(const std::string& token) {
    try {
        const auto decoded = jwt::decode(token);

        // 检查是否过期
        if (decoded.has_expires_at()) {
            if (std::chrono::system_clock::now() > decoded.get_expires_at()) {
                return std::unexpected(JwtError::TokenExpired);
            }
        }

        // 验证签名和 Issuer
        const auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_key_})
            .with_issuer(issuer_);

        verifier.verify(decoded);

        // 提取 user_id
        if (decoded.has_payload_claim(CLAIM_USER_ID)) {
            return decoded.get_payload_claim(CLAIM_USER_ID).as_string();
        }

        SPDLOG_WARN("Token valid but missing claim: {}", CLAIM_USER_ID);
        return std::unexpected(JwtError::FormatInvalid);

    } catch (const std::exception& e) {
        SPDLOG_WARN("JWT Verification Exception: {}", e.what());
        return std::unexpected(JwtError::SignatureInvalid);
    }
}