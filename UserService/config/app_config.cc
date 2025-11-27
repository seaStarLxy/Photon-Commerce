// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "config/app_config.h"

using namespace user_service::config;
using namespace user_service::infrastructure;

AppConfig::AppConfig(const std::string& config_path) {
    try {
        SPDLOG_INFO("Loading configuration from: {}", config_path);
        const YAML::Node root_node = YAML::LoadFile(config_path);
        ParseRedisConfig(root_node);
        ParseDbConfig(root_node);
        ParseJwtConfig(root_node);
    } catch (const YAML::Exception& e) {
        SPDLOG_CRITICAL("Error parsing YAML file '{}': {}", config_path, e.what());
        throw std::runtime_error("Configuration load failed");
    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("Configuration error: {}", e.what());
        throw;
    }
}

void AppConfig::ParseRedisConfig(const YAML::Node& root_node) {
    // 读值
    if (!root_node["redis"]) {
        throw std::runtime_error("Missing 'redis' section");
    }
    const auto& redis_node = root_node["redis"];

    // 取值
    std::string host = redis_node["host"].as<std::string>();
    int port = redis_node["port"].as<int>();

    // 校验
    ValidateNotEmpty(host, "Redis Host");
    ValidatePort(port, "Redis Port");

    // 赋值
    redis_config_.host = host;
    redis_config_.port = std::to_string(port); // RedisConfig 里依然存的是 string
    SPDLOG_INFO("Redis config loaded: {}:{}", redis_config_.host, redis_config_.port);
}

void AppConfig::ParseDbConfig(const YAML::Node& root_node) {
    // 读值
    if (!root_node["postgresql"]) {
        throw std::runtime_error("Missing 'postgresql' section");
    }
    const auto& db_node = root_node["postgresql"];

    // 取值
    std::string host = db_node["host"].as<std::string>();
    int port = db_node["port"].as<int>();
    std::string user = db_node["user"].as<std::string>();
    std::string pwd = db_node["password"].as<std::string>();
    std::string dbname = db_node["dbname"].as<std::string>();
    int pool_size = db_node["pool_size"].as<int>();

    // 校验
    ValidateNotEmpty(host, "DB Host");
    ValidateNotEmpty(user, "DB User");
    ValidateNotEmpty(dbname, "DB Name");
    ValidatePort(port, "DB Port");
    if (pool_size <= 0 || pool_size > 1000) {
        throw std::runtime_error(fmt::format("Config Error: Invalid DB pool_size {}", pool_size));
    }

    // 赋值
    db_pool_config_.conn_str = fmt::format("postgresql://{}:{}@{}:{}/{}",
                                          user, pwd, host, port, dbname);
    db_pool_config_.pool_size = pool_size;
    SPDLOG_INFO("Database config loaded. Host: {}, PoolSize: {}", host, db_pool_config_.pool_size);
}

void AppConfig::ParseJwtConfig(const YAML::Node& root_node) {
    if (!root_node["jwt"]) {
        throw std::runtime_error("Missing 'jwt' section");
    }
    const auto& node = root_node["jwt"];

    const std::string secret = node["secret_key"].as<std::string>();
    const std::string issuer = node["issuer"].as<std::string>();
    const int expire = node["expiration_seconds"].as<int>();

    ValidateNotEmpty(secret, "JWT Secret Key");
    ValidateNotEmpty(issuer, "JWT Issuer");

    if (expire <= 0) {
        throw std::runtime_error("Config Error: JWT expiration_seconds must be positive");
    }

    jwt_config_ = {secret, issuer, expire};
    SPDLOG_INFO("JWT config loaded. Issuer: {}", issuer);
}

void AppConfig::ValidatePort(int port, const std::string& field_name) const {
    if (port <= 0 || port > 65535) {
        throw std::runtime_error(
            fmt::format("Config Error: {} value '{}' is invalid. Must be between 1 and 65535", field_name, port)
        );
    }
}

void AppConfig::ValidateNotEmpty(const std::string& value, const std::string& field_name) const {
    if (value.empty()) {
        throw std::runtime_error(fmt::format("Config Error: '{}' cannot be empty", field_name));
    }
}