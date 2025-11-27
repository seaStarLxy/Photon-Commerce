// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "utils/interface/i_security_util.h"

namespace user_service::util {
    class SecurityUtil: public ISecurityUtil {
    public:
        SecurityUtil() = default;
        ~SecurityUtil() override = default;

        std::string GenerateSalt() override;
        std::string HashPassword(const std::string& raw_password, const std::string& salt) override;
        bool VerifyPassword(const std::string& raw_password, const std::string& salt, const std::string& stored_hash) override;
    };
}