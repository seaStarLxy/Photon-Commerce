// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

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
    SPDLOG_DEBUG("Execute RedisClient Constructor");
    cfg_.addr.host = config.host;
    cfg_.addr.port = config.port;
}

RedisClient::~RedisClient() = default;


boost::asio::awaitable<void> RedisClient::Init() {
    SPDLOG_DEBUG("STARTING to connect redis");
    conn_->async_run(cfg_, boost::asio::detached);
    SPDLOG_DEBUG("next");

    co_await Ping();
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
        // 当下阶段选择抛出，这时候可能 redis 挂了，需要执行降级策略了
        throw;
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
        throw;
    }
}

boost::asio::awaitable<std::optional<std::string>> RedisClient::Get(const std::string& key) {
    SPDLOG_DEBUG("GET {}", key);
    try {
        boost::redis::request req;
        req.push("GET", key);

        boost::redis::response<boost::redis::resp3::node> resp;
        co_await conn_->async_exec(req, resp);

        co_return ExtractResult(std::get<0>(resp), "GET", key);

    } catch (const std::exception& e) {
        SPDLOG_ERROR("Redis GET failed: {}. Key: {}", e.what(), key);
        co_return std::nullopt;
    }
}

boost::asio::awaitable<void> RedisClient::Ping() {
    boost::redis::request req;
    req.push("PING");
    boost::redis::response<boost::redis::resp3::node> resp;

    // async_exec 会自动等待 async_run 建立连接
    try {
        co_await conn_->async_exec(req, resp, boost::asio::use_awaitable);
        const auto result = ExtractResult(std::get<0>(resp), "PING");

        // 针对 PING 的特殊检查
        if (result.has_value() && result.value() == "PONG") {
            SPDLOG_INFO("Redis PING successful.");
            co_return;
        }

        // 如果没抛异常但是也没返回 PONG (ExtractResult 内部已经打过 Error 日志了，这里抛异常即可)
        throw std::runtime_error("Redis PING failed or returned unexpected value");

    } catch (const std::exception& e) {
        SPDLOG_ERROR("Failed to connect to Redis: {}", e.what());
        throw;
    }
}

std::optional<std::string> RedisClient::ExtractResult(const boost::system::result<boost::redis::resp3::node, boost::redis::adapter::error>& result,
            const std::string& command_name, const std::string& key_context) const {

    // A. 检查系统级错误 (比如网络断开)
    if (result.has_error()) {
        SPDLOG_ERROR("Redis {} failed with system error: {}. Context: {}",
            command_name, result.error().diagnostic, key_context);
        return std::nullopt;
    }

    const auto& node = result.value();

    // B. 检查是否是字符串 (Blob String 或 Simple String)，这是 Happy Path
    // 注意：PONG 是 simple_string，GET 的值通常是 blob_string
    if (node.data_type == boost::redis::resp3::type::blob_string || node.data_type == boost::redis::resp3::type::simple_string) {
        return node.value;
    }

    // C. 检查 Null (Key 不存在)
    if (node.data_type == boost::redis::resp3::type::null) {
        SPDLOG_DEBUG("Redis {} key '{}' not found (NULL).", command_name, key_context);
        return std::nullopt;
    }

    // D. 检查语义错误 (比如命令参数不对)
    if (node.data_type == boost::redis::resp3::type::simple_error ||
        node.data_type == boost::redis::resp3::type::blob_error) {
        SPDLOG_ERROR("Redis {} returned Redis-Error: {}. Context: {}", command_name, node.value, key_context);
        return std::nullopt;
        }

    // E. 其他未预期的类型 (比如返回了 Array 或 Int)
    SPDLOG_WARN("Redis {} returned unexpected type: {}. Context: {}",
        command_name, static_cast<int>(node.data_type), key_context);
    return std::nullopt;
}