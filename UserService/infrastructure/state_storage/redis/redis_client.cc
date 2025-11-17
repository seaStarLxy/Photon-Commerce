// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "redis_client.h"
#include <spdlog/spdlog.h>
#include <boost/redis/request.hpp>
#include <boost/redis/response.hpp>
#include <boost/redis/resp3/node.hpp>
#include <boost/redis/resp3/type.hpp>

using namespace user_service::infrastructure;

RedisClient::RedisClient(const std::shared_ptr<boost::asio::io_context>& ioc, const RedisConfig& config):
    ioc_(ioc), conn_(std::make_shared<boost::redis::connection>(boost::asio::make_strand(ioc->get_executor())))
{
    cfg_.addr.host = config.host;
    cfg_.addr.port = config.port;
}

RedisClient::~RedisClient() = default;


void RedisClient::Run() const {
    SPDLOG_DEBUG("STARTING to connect redis");
    conn_->async_run(cfg_, boost::asio::detached);
    SPDLOG_DEBUG("next");
}

boost::asio::awaitable<void> RedisClient::Set(const std::string& key, const std::string& value) {
    SPDLOG_DEBUG("SET {}: {}", key, value);
    try {
        // 创建 SET 请求
        boost::redis::request req;
        req.push("SET", key, value);
        // 异步执行请求
        co_await conn_->async_exec(req, boost::redis::ignore);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Redis SET failed: {}. Key: {}, Value: {}", e.what(), key, value);
    }
}

boost::asio::awaitable<void> RedisClient::Set(const std::string& key, const std::string& value, const std::chrono::seconds& expiry)
{
    SPDLOG_DEBUG("SET {}: {} (Expiry: {}s)", key, value, expiry.count());
    try {
        boost::redis::request req;
        // SET key value EX seconds
        req.push("SET", key, value, "EX", std::to_string(expiry.count()));
        co_await conn_->async_exec(req, boost::redis::ignore);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Redis SETEX failed: {}. Key: {}, Value: {}, Expiry: {}", e.what(), key, value, expiry.count());
    }
}

boost::asio::awaitable<std::optional<std::string>> RedisClient::Get(const std::string& key) {
    SPDLOG_DEBUG("GET {}", key);
    try {
        boost::redis::request req;
        req.push("GET", key);

        boost::redis::response<boost::redis::resp3::node> resp;
        co_await conn_->async_exec(req, resp);

        auto& result = std::get<0>(resp);

        // 检查系统级错误
        if (result.has_error()) {
            SPDLOG_ERROR("Redis GET failed with system error: {}. Key: {}", result.error().diagnostic, key);
            co_return std::nullopt;
        }

        const auto& node = result.value();

        // 找到了
        if (node.data_type == boost::redis::resp3::type::blob_string) {
            co_return node.value;
        }

        // 没找到
        if (node.data_type == boost::redis::resp3::type::null) {
            SPDLOG_DEBUG("Redis GET key '{}' not found.", key);
            co_return std::nullopt;
        }

        // 语义错误
        if (node.data_type == boost::redis::resp3::type::simple_error) {
            SPDLOG_ERROR("Redis GET error for key '{}': {}", key, node.value);
            co_return std::nullopt;
        }

        SPDLOG_WARN("Redis GET for key '{}' returned unexpected type: {}", key, static_cast<int>(node.data_type));
        co_return std::nullopt;

    } catch (const std::exception& e) {
        SPDLOG_ERROR("Redis GET failed: {}. Key: {}", e.what(), key);
        co_return std::nullopt;
    }
}