// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "redis_client.h"
#include <spdlog/spdlog.h>

using namespace user_service::infrastructure;

RedisClient::RedisClient(const std::shared_ptr<boost::asio::io_context>& ioc, const RedisConfig& config):
    ioc_(ioc), conn_(std::make_shared<boost::redis::connection>(boost::asio::make_strand(ioc->get_executor())))
{
    cfg_.addr.host = config.host;
    cfg_.addr.port = config.port;
}

RedisClient::~RedisClient() = default;


void RedisClient::Run() {
    SPDLOG_DEBUG("STARTING to connect redis");
    conn_->async_run(cfg_, boost::asio::detached);
    SPDLOG_DEBUG("next");
}

boost::asio::awaitable<void> RedisClient::Set(const std::string& key, const std::string& value) {
    SPDLOG_DEBUG("{}: {}", key, value);
    co_return;
}

boost::asio::awaitable<std::optional<std::string>> RedisClient::Get(const std::string& key) {
    co_return std::nullopt;
}