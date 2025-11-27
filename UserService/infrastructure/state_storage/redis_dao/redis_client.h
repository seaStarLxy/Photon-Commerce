// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <string>
#include <expected>
#include <boost/redis/connection.hpp>
#include <boost/asio.hpp>

namespace user_service::infrastructure {

    enum class RedisErrorType {
        SystemError,
        CommandError,
        ProtocolError
    };

    struct RedisError {
        RedisErrorType type;
        std::string message;
    };

    struct RedisConfig {
        std::string host;
        std::string port;
    };

    class RedisClient {
    public:
        RedisClient(const std::shared_ptr<boost::asio::io_context>& ioc, const RedisConfig& config);
        ~RedisClient();
        boost::asio::awaitable<void> Init();
        boost::asio::awaitable<std::expected<void, RedisError>> Set(const std::string& key, const std::string& value) const;
        boost::asio::awaitable<std::expected<void, RedisError>> Set(const std::string& key, const std::string& value, const std::chrono::seconds& expiry) const;
        boost::asio::awaitable<std::expected<std::optional<std::string>, RedisError>> Get(const std::string& key) const;
    private:
        /*
         * 注意：此时 Ping 只是在 Init 中被调用，理论上没有线程安全问题，但是为了防止后续被多线程环境使用，对函数内部进行了安全处理
         */
        // 等待联通
        boost::asio::awaitable<std::expected<void, RedisError>> Ping() const;

        /*
         * 注意：这个函数目前工作量不大，所以和 conn->async_exec 一起在 strand 串行区进行处理。
         * 若后续此函数变重，最好移出串行区，让计算型线程来并发处理。
         * 原则：计算开销 > 上下文切换的开销 时，移出串行区
         */
        // 结果解析
        [[nodiscard]] static std::expected<std::optional<std::string>, RedisError> ExtractResult(
            const boost::system::result<boost::redis::resp3::node, boost::redis::adapter::error>& result,
            const std::string& command_name, const std::string& key_context= "");

        const std::shared_ptr<boost::asio::io_context> ioc_;
        std::shared_ptr<boost::redis::connection> conn_;
        boost::redis::config cfg_;
    };
}