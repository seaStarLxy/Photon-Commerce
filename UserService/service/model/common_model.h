// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <string>
#include <cstdint>

namespace user_service::service {
    enum class ErrorCode: std::int32_t {
        SUCCESS = 0,

        // 通用错误
        INTERNAL_ERROR = 1000,
        INVALID_ARGUMENT = 1001,
        UNAUTHORIZED = 1002,

        // 用户相关错误 (2000+)
        USER_NOT_FOUND = 2001,
        USER_ALREADY_EXISTS = 2002,
        PASSWORD_INCORRECT = 2003,

        // 验证码相关 (3000+)
        VERIFICATION_CODE_EXPIRED = 3001,
        VERIFICATION_CODE_MISMATCH = 3002
    };
    struct CommonStatus
    {
        ErrorCode code;
        std::string message;
        CommonStatus(): code(ErrorCode::SUCCESS), message("") {}
        CommonStatus(ErrorCode code, const std::string& msg): code(code), message(msg) {}
        static CommonStatus Success() {
            return {ErrorCode::SUCCESS, "操作成功"};
        }
    };

    // class BusinessException : public std::runtime_error {
    // public:
    //     BusinessException(const ErrorCode code, const std::string& message)
    //         : std::runtime_error(message), code_(code) {}
    //
    //     ErrorCode GetCode() const {
    //         return code_;
    //     }
    //
    // private:
    //     ErrorCode code_;
    // };
}
