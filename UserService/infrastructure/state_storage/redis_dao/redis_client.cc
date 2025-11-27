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
    // 启动连接
    conn_->async_run(cfg_, boost::asio::detached);
    SPDLOG_DEBUG("Redis async_run started");

    const auto ping_res = co_await Ping();
    if (!ping_res.has_value()) {
        std::string err_msg = fmt::format("Redis Init failed: {}", ping_res.error().message);
        SPDLOG_CRITICAL("{}", err_msg);
        // 启动阶段抛异常
        throw std::runtime_error(err_msg);
    }

    SPDLOG_INFO("Redis Init successfully.");
}

boost::asio::awaitable<std::expected<void, RedisError>> RedisClient::Set(const std::string& key, const std::string& value) const {
    SPDLOG_DEBUG("SET {}: {}", key, value);
    try {
        boost::redis::request req;
        req.push("SET", key, value);

        // 进入串行区
        co_await boost::asio::post(conn_->get_executor(), boost::asio::use_awaitable);
        co_await conn_->async_exec(req, boost::redis::ignore, boost::asio::use_awaitable);

        co_return std::expected<void, RedisError>();
    } catch (const std::exception& e) {
        // 与 redis 断开连接
        SPDLOG_ERROR("Redis SET failed: {}. Key: {}, Value: {}", e.what(), key, value);
        co_return std::unexpected(RedisError{
            RedisErrorType::SystemError,
            fmt::format("SET exception: {}", e.what())
        });
    }
}

boost::asio::awaitable<std::expected<void, RedisError>> RedisClient::Set(const std::string& key,
    const std::string& value, const std::chrono::seconds& expiry) const {
    SPDLOG_DEBUG("SET {}: {} (Expiry: {}s)", key, value, expiry.count());
    try {
        boost::redis::request req;
        if (expiry.count() > 0) {
            req.push("SET", key, value, "EX", std::to_string(expiry.count()));
        } else {
            req.push("SET", key, value);
        }

        // 进入串行区
        co_await boost::asio::post(conn_->get_executor(), boost::asio::use_awaitable);
        co_await conn_->async_exec(req, boost::redis::ignore, boost::asio::use_awaitable);

        co_return std::expected<void, RedisError>();
    } catch (const std::exception& e) {
        // 与 redis 断开连接
        SPDLOG_ERROR("Redis SET Exception: {}", e.what());
        co_return std::unexpected(RedisError{
            RedisErrorType::SystemError,
            fmt::format("SET exception: {} (Key: {})", e.what(), key)
        });
    }
}

boost::asio::awaitable<std::expected<std::optional<std::string>, RedisError>> RedisClient::Get(const std::string& key) const {
    SPDLOG_DEBUG("GET {}", key);
    try {
        boost::redis::request req;
        req.push("GET", key);

        boost::redis::response<boost::redis::resp3::node> resp;

        // 进入串行区
        co_await boost::asio::post(conn_->get_executor(), boost::asio::use_awaitable);
        co_await conn_->async_exec(req, resp, boost::asio::use_awaitable);

        co_return ExtractResult(std::get<0>(resp), "GET", key);
    } catch (const std::exception& e) {
        // 与 redis 断开连接
        SPDLOG_ERROR("Redis GET Exception: {}", e.what());
        co_return std::unexpected(RedisError{
            RedisErrorType::SystemError,
            fmt::format("GET exception: {} (Key: {})", e.what(), key)
        });
    }
}

boost::asio::awaitable<std::expected<void, RedisError>> RedisClient::Ping() const {
    try {
        boost::redis::request req;
        req.push("PING");

        boost::redis::response<boost::redis::resp3::node> resp;

        // 进入串行区
        co_await boost::asio::post(conn_->get_executor(), boost::asio::use_awaitable);
        co_await conn_->async_exec(req, resp, boost::asio::use_awaitable);

        auto result = ExtractResult(std::get<0>(resp), "PING", "Init");

        if (!result.has_value()) {
            co_return std::unexpected(result.error());
        }

        // 检查 PING 返回值
        const auto& opt_val = result.value();

        // PING 必须返回 "PONG"
        if (opt_val.has_value() && opt_val.value() == "PONG") {
            SPDLOG_DEBUG("Redis PING successful.");
            co_return std::expected<void, RedisError>(); // 成功
        }

        // 走到这里说明返回了 Null 或者不是 PONG
        std::string val = opt_val.value_or("NULL");
        std::string msg = fmt::format("Redis PING protocol error. Expected 'PONG', got '{}'", val);
        SPDLOG_ERROR("{}", msg);

        co_return std::unexpected(RedisError{RedisErrorType::ProtocolError, msg});

    } catch (const std::exception& e) {
        std::string msg = fmt::format("Redis PING exception: {}", e.what());
        SPDLOG_ERROR("{}", msg);
        co_return std::unexpected(RedisError{RedisErrorType::SystemError, msg});
    }
}

std::expected<std::optional<std::string>, RedisError> RedisClient::ExtractResult(
    const boost::system::result<boost::redis::resp3::node, boost::redis::adapter::error>& result,
    const std::string& command_name, const std::string& key_context) {

    // A: 系统级错误 (System Error)
    if (result.has_error()) {
        std::string msg = fmt::format("Redis {} failed. Error: {}. Context: {}",
                                      command_name, result.error().diagnostic, key_context);
        SPDLOG_ERROR("{}", msg);
        return std::unexpected(RedisError{RedisErrorType::SystemError, msg});
    }

    const auto& node = result.value();

    // B. 检查是否是字符串 (Blob String 或 Simple String)，这是 Happy Path
    // 注意：PONG 是 simple_string，GET 的值通常是 blob_string
    if (node.data_type == boost::redis::resp3::type::blob_string || node.data_type == boost::redis::resp3::type::simple_string) {
        return std::make_optional(node.value);
    }

    // C: Null (Cache Miss) 正常业务逻辑，不是 Error
    if (node.data_type == boost::redis::resp3::type::null) {
        return std::nullopt;
    }

    // D: 检查语义错误 (比如命令参数不对)
    if (node.data_type == boost::redis::resp3::type::simple_error ||
        node.data_type == boost::redis::resp3::type::blob_error) {

        std::string msg = fmt::format("Redis {} Command Error: {}. Context: {}",
                                      command_name, node.value, key_context);
        SPDLOG_ERROR("{}", msg);
        return std::unexpected(RedisError{RedisErrorType::CommandError, msg});
        }

    // E: 其他未预期的类型 (比如返回了 Array 或 Int)
    std::string msg = fmt::format("Redis {} Protocol Error: Unexpected type {}. Context: {}",
                                  command_name, static_cast<int>(node.data_type), key_context);
    SPDLOG_WARN("{}", msg);
    return std::unexpected(RedisError{RedisErrorType::ProtocolError, msg});
}