// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <string>

namespace user_service::service {
    enum CodeUsage {
        UNKNOWN = 0,
        USER_REGISTER = 1,
        USER_LOGIN = 2,
        RESET_PASSWORD = 3,
    };
    struct SendCodeRequest {
        std::string phone_number;
        CodeUsage usage;
    };
    struct SendCodeResponse {
        bool status;
    };
    struct LoginByPasswordRequest {
        std::string username;
        std::string password;
    };
    struct LoginByCodeRequest {
        std::string phone_number;
        std::string code;
    };
    struct LoginResult {
        std::string token;
        std::string user_id;
    };
}
