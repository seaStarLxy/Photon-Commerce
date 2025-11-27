// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <string>

namespace user_service::util {
    class ISecurityUtil {
    public:
        virtual ~ISecurityUtil() = default;
        // 生成随机盐值
        virtual std::string GenerateSalt() = 0;

        // 加密
        virtual std::string HashPassword(const std::string& raw_password, const std::string& salt) = 0;

        // 验证密码是否匹配
        virtual bool VerifyPassword(const std::string& raw_password, const std::string& salt, const std::string& stored_hash) = 0;
    };
}