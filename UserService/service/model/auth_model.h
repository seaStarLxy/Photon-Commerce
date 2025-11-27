// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "service/model/common_model.h"
#include <string>

namespace user_service::service {
    // 发送验证码
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
        CommonStatus status;
    };

    // 密码登录
    struct LoginByPasswordRequest {
        std::string user_id;
        std::string password;
    };
    // 验证码登录
    struct LoginByCodeRequest {
        std::string phone_number;
        std::string code;
    };
    // 登录结果
    struct LoginResult {
        CommonStatus status;
        std::string token;
        std::string user_id;
    };
}
