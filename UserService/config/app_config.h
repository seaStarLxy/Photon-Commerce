// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <string>
#include <yaml-cpp/yaml.h>
#include "infrastructure/state_storage/redis_dao/redis_client.h"
#include "infrastructure/persistence/postgresql/include/async_connection_pool.h"

namespace user_service::config {
    class AppConfig {
    public:
        explicit AppConfig(const std::string& config_path);

        // 配置对象不允许拷贝、赋值
        AppConfig(const AppConfig&) = delete;
        AppConfig& operator=(const AppConfig&) = delete;

        infrastructure::RedisConfig GetRedisConfig() const;
        infrastructure::DbPoolConfig GetDBPoolConfig() const;

    private:
        // YAML::Node，代表配置树的一个节点
        void ParseRedisConfig(const YAML::Node& root_node);
        void ParseDbConfig(const YAML::Node& root_node);

        /* 校验逻辑 */
        void ValidatePort(int port, const std::string& field_name) const;
        void ValidateNotEmpty(const std::string& value, const std::string& field_name) const;

        infrastructure::RedisConfig redis_config;
        infrastructure::DbPoolConfig db_pool_config;
    };
}