// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <expected>
#include <string>

namespace user_service::util {

    enum class JwtError {
        TokenExpired,
        SignatureInvalid,
        FormatInvalid,
        InternalError
    };

    class IJwtUtil {
    public:
        virtual ~IJwtUtil() = default;
        // 生成 Token
        virtual std::string GenerateToken(const std::string& user_id) = 0;

        // 验证 Token, 返回解析出的 user_id
        virtual std::expected<std::string, JwtError> VerifyToken(const std::string& token) = 0;
    };
}