// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "../include/user_repository.h"

using namespace user_service::infrastructure;
using namespace user_service::domain;

UserRepository::UserRepository(const std::shared_ptr<UserDao>& user_dao, const std::shared_ptr<RedisClient>& redis_client):
    user_dao_(user_dao), redis_client_(redis_client) {

}

UserRepository::~UserRepository() = default;

boost::asio::awaitable<std::expected<void, DbError>> UserRepository::CreateUser(const User& user) {
    co_return co_await user_dao_->CreateUser(user);
}

boost::asio::awaitable<std::expected<std::optional<User>, DbError>> UserRepository::GetUserById(const std::string& id) {
    // mock 数据
    // auto mock_user = user_service::domain::User::Create(
    //     id,                                      // 使用参数 id，保持一致性
    //     "+8613812345678",                        // 模拟手机号
    //     "e10adc3949ba59abbe56e057f20f883e",      // 模拟 Password Hash (MD5/Argon2 string)
    //     "random_salt_string_16bytes"             // 模拟 Salt
    // );
    // mock_user.UpdateProfile(
    //     "Benchmark_User_Name",                   // username
    //     "benchmark_user@nus.edu.sg",             // email
    //     "https://oss.example.com/avatars/default.png" // avatar_url
    // );
    // co_return std::optional<user_service::domain::User>(std::move(mock_user));
    const std::string cache_key = "user:info:" + id;

    // 尝试读缓存
    const auto redis_res = co_await redis_client_->Get(cache_key);

    if (redis_res.has_value()) {    // Redis 调用成功
        const auto& opt_val = redis_res.value();

        if (opt_val.has_value()) {  // 缓存命中
            // 使用不抛异常的 parse 接口
            // 参数2: callback=nullptr, 参数3: allow_exceptions=false
            nlohmann::json j = nlohmann::json::parse(opt_val.value(), nullptr, false);
            if (!j.is_discarded()) {
                auto user_opt = User::FromJson(j); // User::FromJson 内部处理了字段缺失异常
                if (user_opt.has_value()) {
                    SPDLOG_DEBUG("Cache HIT for user: {}", id);
                    co_return user_opt;
                }
            }
            // 解析失败（数据损坏或版本不兼容），当做缓存未命中
            SPDLOG_WARN("Cache invalid for user: {}, refreshing from DB", id);
        }
    } else {    // redis 调用失败
        // 降级策略：只打日志，继续查库，保证高可用
        SPDLOG_WARN("Redis error ignored in GetUserById: {}", redis_res.error().message);
    }

    // 缓存未命中，查数据库
    auto db_result_exp = co_await user_dao_->GetUserById(id);

    // DB 出错直接返回
    if (!db_result_exp.has_value()) {
        co_return db_result_exp;
    }

    const auto& user_opt = db_result_exp.value();

    // 查库成功，回填缓存
    if (user_opt.has_value()) {
        try {
            // ToJson().dump() 可能会因为内存耗尽抛异常，防一下比较稳妥
            const std::string json_str = user_opt.value().ToJson().dump();

            // Set 现在返回 expected，不会抛网络异常了
            const auto set_res = co_await redis_client_->Set(cache_key, json_str, std::chrono::seconds(3600));

            if (!set_res.has_value()) {
                SPDLOG_WARN("Failed to populate cache for user {}: {}", id, set_res.error().message);
            } else {
                SPDLOG_DEBUG("Cache MISS. Populated redis for user: {}", id);
            }
        } catch (const std::exception& e) {
            // 仅捕获 JSON 序列化可能的异常
            SPDLOG_WARN("Serialization failed for user {}: {}", id, e.what());
        }
    }
    co_return db_result_exp;
}

boost::asio::awaitable<std::expected<std::optional<User>, DbError>> UserRepository::GetUserByPhoneNumber(const std::string& phoneNumber) {
    co_return co_await user_dao_->GetUserByPhoneNumber(phoneNumber);
}