// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <string>
#include <yaml-cpp/yaml.h>
#include "server/user_service_server.h"
#include "service_registry/include/consul_registry.h"
#include "infrastructure/state_storage/redis_dao/redis_client.h"
#include "infrastructure/persistence/postgresql/include/async_connection_pool.h"
#include "utils/include/jwt_util.h"

namespace user_service::config {
    class AppConfig {
    public:
        explicit AppConfig(const std::string& config_path);

        // 配置对象不允许拷贝、赋值
        AppConfig(const AppConfig&) = delete;
        AppConfig& operator=(const AppConfig&) = delete;

        server::ServerConfig GetServerConfig() const { return server_config_; }
        server::RpcLimitsConfig GetRpcLimitsConfig() const { return rpc_limits_config_; }
        registry::ConsulConfig GetConsulConfig() const { return consul_config_; }
        infrastructure::RedisConfig GetRedisConfig() const { return redis_config_; };
        infrastructure::DbPoolConfig GetDBPoolConfig() const { return db_pool_config_; };
        util::JwtConfig GetJwtConfig() const { return jwt_config_; }

    private:
        // YAML::Node，代表配置树的一个节点
        void ParseServerConfig(const YAML::Node& root_node);
        void ParseConsulConfig(const YAML::Node& root_node);
        void ParseRpcLimitsConfig(const YAML::Node& root_node);
        void ParseRedisConfig(const YAML::Node& root_node);
        void ParseDbConfig(const YAML::Node& root_node);
        void ParseJwtConfig(const YAML::Node& root_node);

        /* 校验逻辑 */
        static void ValidatePort(int port, const std::string& field_name);
        static void ValidateNotEmpty(const std::string& value, const std::string& field_name);

        server::ServerConfig server_config_;
        server::RpcLimitsConfig rpc_limits_config_;
        registry::ConsulConfig consul_config_;
        infrastructure::RedisConfig redis_config_;
        infrastructure::DbPoolConfig db_pool_config_;
        util::JwtConfig jwt_config_;
    };
}