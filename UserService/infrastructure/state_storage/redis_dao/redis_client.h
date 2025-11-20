// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <string>
#include <boost/redis/connection.hpp>
#include <boost/asio.hpp>

namespace user_service::infrastructure {
    struct RedisConfig {
        std::string host;
        std::string port;
    };
    class RedisClient {
    public:
        RedisClient(const std::shared_ptr<boost::asio::io_context>& ioc, const RedisConfig& config);
        ~RedisClient();
        boost::asio::awaitable<void> Init();
        boost::asio::awaitable<void> Set(const std::string& key, const std::string& value);
        boost::asio::awaitable<void> Set(const std::string& key, const std::string& value, const std::chrono::seconds& expiry);
        boost::asio::awaitable<std::optional<std::string>> Get(const std::string& key);
    private:
        // 等待联通
        boost::asio::awaitable<void> Ping();
        // 结果解析
        [[nodiscard]] std::optional<std::string> ExtractResult(const boost::system::result<boost::redis::resp3::node, boost::redis::adapter::error>& result,
            const std::string& command_name, const std::string& key_context= "") const;

        const std::shared_ptr<boost::asio::io_context> ioc_;
        std::shared_ptr<boost::redis::connection> conn_;
        boost::redis::config cfg_;
    };
}