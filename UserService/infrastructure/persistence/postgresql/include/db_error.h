// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <string>

namespace user_service::infrastructure {
    enum class DbErrorType {
        NetworkError,
        SqlExecutionError,  // 执行时错误
    };
    struct DbError {
        DbErrorType type;
        std::string pg_error_message;
        std::string sql_state;  // PGSQL 的错误代码，比如 "23505" 代表唯一键冲突
    };
}