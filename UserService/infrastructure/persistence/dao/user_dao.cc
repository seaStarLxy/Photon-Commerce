// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "infrastructure/persistence/dao/user_dao.h"
#include <spdlog/spdlog.h>

using namespace user_service::infrastructure;
using namespace user_service::domain;

UserDao::UserDao(const std::shared_ptr<AsyncConnectionPool>& pool): pool_(pool) {

}

UserDao::~UserDao() = default;

boost::asio::awaitable<std::expected<void, DbError>> UserDao::CreateUser(const User& user) const {
    const auto conn = co_await pool_->GetConnection();

    const std::string sql = "INSERT INTO users (id, phone_number, username, email, password_hash, salt, avatar_url, status) "
                            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8)";

    const std::vector<std::string> params = {
        user.GetId(),
        user.GetPhoneNumber(),
        user.GetUsername().value_or(""),
        user.GetEmail().value_or(""),
        user.GetPasswordHash(),
        user.GetSalt(),
        user.GetAvatarUrl().value_or(""),
        std::to_string(user.GetStatusValue())
    };

    const auto result_exp = co_await conn->AsyncExecParams(sql, params);

    if (!result_exp.has_value()) {
        co_return std::unexpected(result_exp.error());
    }
    co_return std::expected<void, DbError>{};
}

boost::asio::awaitable<std::expected<std::optional<User>, DbError>> UserDao::GetUserById(const std::string& id) {
    SPDLOG_DEBUG("{}", id);
    const auto conn = co_await pool_->GetConnection();

    const std::string sql = "SELECT id, phone_number, username, email, password_hash, salt, avatar_url, status, created_at "
                            "FROM users WHERE id = $1 AND deleted_at IS NULL LIMIT 1";
    const std::vector<std::string> params = { id };

    auto result_exp = co_await conn->AsyncExecParams(sql, params);

    if (!result_exp.has_value()) {
        co_return std::unexpected(result_exp.error());
    }

    const auto result_ptr = std::move(result_exp.value());

    if (PQntuples(result_ptr.get()) == 0) {
        co_return std::nullopt;
    }

    co_return MapRowToUser(result_ptr.get(), 0);
}

boost::asio::awaitable<std::expected<std::optional<User>, DbError>> UserDao::GetUserByPhoneNumber(const std::string& phone_number) {
    const auto conn = co_await pool_->GetConnection();
    const std::string sql = "SELECT id, phone_number, username, email, password_hash, salt, avatar_url, status, created_at "
                  "FROM users WHERE phone_number = $1 AND deleted_at IS NULL LIMIT 1";
    // 避免把临时对象传给协程
    const std::vector<std::string> params = { phone_number };
    auto result_exp = co_await conn->AsyncExecParams(sql, params);

    if (!result_exp.has_value()) {
        co_return std::unexpected(result_exp.error());
    }

    const auto result_ptr = std::move(result_exp.value());

    // 检查是否有数据
    if (PQntuples(result_ptr.get()) == 0) {
        // 显示构造，避免 CIE
        std::optional<User> empty_opt = std::nullopt;
        std::expected<std::optional<User>, DbError> ret = std::move(empty_opt);
        co_return ret;
    }

    // 查到数据，映射为 domain 对象
    User user = MapRowToUser(result_ptr.get(), 0);
    SPDLOG_DEBUG("{}", user.ToString());
    std::optional<User> opt_user = std::make_optional(std::move(user));
    std::expected<std::optional<User>, DbError> ret = std::move(opt_user);
    co_return ret;
}


User UserDao::MapRowToUser(const PGresult* res, int row) {
    User user;

    user.id_ = PQgetvalue(res, row, 0);
    user.phone_number_ = PQgetvalue(res, row, 1);

    if (!PQgetisnull(res, row, 2)) user.username_ = PQgetvalue(res, row, 2);
    if (!PQgetisnull(res, row, 3)) user.email_ = PQgetvalue(res, row, 3);

    user.password_hash_ = PQgetvalue(res, row, 4);
    user.salt_ = PQgetvalue(res, row, 5);

    if (!PQgetisnull(res, row, 6)) user.avatar_url_ = PQgetvalue(res, row, 6);
    user.status_ = static_cast<UserStatus>(std::stoi(PQgetvalue(res, row, 7)));

    user.created_at_ = ParsePostgresTimestamp(PQgetvalue(res, row, 8));

    return user;
}

std::chrono::system_clock::time_point UserDao::ParsePostgresTimestamp(const char* timestamp_str) {
    // 空指针或空字符串检查
    if (timestamp_str == nullptr || timestamp_str[0] == '\0') {
        return std::chrono::system_clock::from_time_t(0);
    }

    std::tm tm = {};
    // 使用 Linux C 函数解析 "YYYY-MM-DD HH:MM:SS"
    const char* ptr = strptime(timestamp_str, "%Y-%m-%d %H:%M:%S", &tm);
    if (ptr == nullptr) {
        SPDLOG_WARN("Failed to parse timestamp: {}", timestamp_str);
        return std::chrono::system_clock::from_time_t(0);
    }
    // Postgres 存的是需要时 UTC 时间
    const time_t t = timegm(&tm);
    return std::chrono::system_clock::from_time_t(t);
}