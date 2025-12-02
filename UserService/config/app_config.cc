// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "config/app_config.h"

using namespace user_service::config;
using namespace user_service::infrastructure;

AppConfig::AppConfig(const std::string& config_path) {
    try {
        SPDLOG_INFO("Loading configuration from: {}", config_path);
        const YAML::Node root_node = YAML::LoadFile(config_path);
        ParseServerConfig(root_node);
        ParseConsulConfig(root_node);
        ParseRpcLimitsConfig(root_node);
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

void AppConfig::ParseServerConfig(const YAML::Node& root_node) {
    // 一级节点检查
    if (!root_node["server"]) throw std::runtime_error("Missing 'server' section");
    const auto& node = root_node["server"];

    // 二级节点检查
    if (!node["name"]) throw std::runtime_error("Config Error: Missing 'server.name'");
    if (!node["bind_ip"]) throw std::runtime_error("Config Error: Missing 'server.bind_ip'");
    if (!node["port"]) throw std::runtime_error("Config Error: Missing 'server.port'");
    if (!node["registry_ip"]) throw std::runtime_error("Config Error: Missing 'server.registry_ip'");
    if (!node["listen_threads"]) throw std::runtime_error("Config Error: Missing 'server.listen_threads'");
    if (!node["worker_threads"]) throw std::runtime_error("Config Error: Missing 'server.worker_threads'");

    // 取值
    const std::string name = node["name"].as<std::string>();
    const std::string bind_ip = node["bind_ip"].as<std::string>();
    const int port = node["port"].as<int>();
    const std::string registry_ip = node["registry_ip"].as<std::string>();
    const int listen_threads = node["listen_threads"].as<int>();
    const int worker_threads = node["worker_threads"].as<int>();

    // 校验
    ValidateNotEmpty(name, "Server Name");
    ValidateNotEmpty(bind_ip, "Server Bind IP");
    ValidateNotEmpty(registry_ip, "Server Registry IP");
    ValidatePort(port, "Server Port");
    if (listen_threads < 0) {
        throw std::runtime_error(fmt::format("Config Error: Invalid listen_threads {}", listen_threads));
    }
    if (worker_threads < 0) {
        throw std::runtime_error(fmt::format("Config Error: Invalid worker_threads {}", worker_threads));
    }

    const unsigned hw_conc = std::thread::hardware_concurrency();
    // 极端情况下获取不到(返回0)则兜底为2
    const int auto_cpu_cores = (hw_conc > 0) ? static_cast<int>(hw_conc) : 2;

    // 赋值 ServerConfig
    server_config_.bind_ip = bind_ip;
    server_config_.port = port;
    server_config_.listen_threads = (listen_threads == 0) ? auto_cpu_cores : listen_threads;
    server_config_.worker_threads = (worker_threads == 0) ? auto_cpu_cores : worker_threads;
    server_config_.register_info.service_name = name;
    server_config_.register_info.ip = registry_ip;
    server_config_.register_info.port = port;
    server_config_.register_info.tags = {"v1", "stable"}; // 暂时使用默认标签

    SPDLOG_INFO("Server config loaded. Name: {}, Bind: {}:{}, RegistryIP: {}",
        name, bind_ip, port, registry_ip);
}

void AppConfig::ParseConsulConfig(const YAML::Node& root_node) {
    // 一级节点检查
    if (!root_node["consul"]) throw std::runtime_error("Missing 'consul' section");
    const auto& node = root_node["consul"];

    // 二级节点检查
    if (!node["host"]) throw std::runtime_error("Config Error: Missing 'consul.host'");
    if (!node["port"]) throw std::runtime_error("Config Error: Missing 'consul.port'");
    if (!node["check_interval"]) throw std::runtime_error("Config Error: Missing 'consul.check_interval'");
    if (!node["check_timeout"]) throw std::runtime_error("Config Error: Missing 'consul.check_timeout'");

    // 取值
    const std::string host = node["host"].as<std::string>();
    const int port = node["port"].as<int>();
    const int check_interval = node["check_interval"].as<int>();
    const int check_timeout = node["check_timeout"].as<int>();

    // 校验
    ValidateNotEmpty(host, "Consul Host");
    ValidatePort(port, "Consul Port");
    if (check_interval <= 0) throw std::runtime_error("Config Error: check_interval must be positive");
    if (check_timeout <= 0) throw std::runtime_error("Config Error: check_timeout must be positive");

    // 赋值 ConsulConnectConfig
    consul_config_.ip = host;
    consul_config_.port = port;

    // 赋值 RegisterConfig
    server_config_.register_info.check_interval_seconds = check_interval;
    server_config_.register_info.check_timeout_seconds = check_timeout;

    SPDLOG_INFO("Consul config loaded. Host: {}:{}, HealthCheck: {}s/{}s", host, port, check_interval, check_timeout);
}

void AppConfig::ParseRpcLimitsConfig(const YAML::Node& root_node) {
    // 一级节点检查
    if (!root_node["rpc_limits"]) throw std::runtime_error("Missing 'rpc_limits' section");
    const auto& node = root_node["rpc_limits"];

    // 二级节点检查
    if (!node["register"]) throw std::runtime_error("Config Error: Missing 'rpc_limits.register'");
    if (!node["send_code"]) throw std::runtime_error("Config Error: Missing 'rpc_limits.send_code'");
    if (!node["login_pw"]) throw std::runtime_error("Config Error: Missing 'rpc_limits.login_pw'");
    if (!node["login_code"]) throw std::runtime_error("Config Error: Missing 'rpc_limits.login_code'");
    if (!node["get_user_info"]) throw std::runtime_error("Config Error: Missing 'rpc_limits.get_user_info'");

    // 取值
    const int reg = node["register"].as<int>();
    const int send_code = node["send_code"].as<int>();
    const int login_pw = node["login_pw"].as<int>();
    const int login_code = node["login_code"].as<int>();
    const int get_info = node["get_user_info"].as<int>();

    // 校验 (calldata 数目大于0)
    if (reg <= 0 || send_code <= 0 || login_pw <= 0 || login_code <= 0 || get_info <= 0) {
        throw std::runtime_error("Config Error: All RPC limits must be positive integers");
    }

    // 赋值
    rpc_limits_config_.register_num = reg;
    rpc_limits_config_.send_code_num = send_code;
    rpc_limits_config_.login_pw_num = login_pw;
    rpc_limits_config_.login_code_num = login_code;
    rpc_limits_config_.get_user_info_num = get_info;

    SPDLOG_INFO("RPC Limits loaded. Register: {}, GetUserInfo: {}", reg, get_info);
}

void AppConfig::ParseRedisConfig(const YAML::Node& root_node) {
    // 一级节点检查
    if (!root_node["redis"]) throw std::runtime_error("Missing 'redis' section");
    const auto& redis_node = root_node["redis"];
    // 二级节点检查
    if (!redis_node["host"]) throw std::runtime_error("Config Error: Missing 'redis.host'");
    if (!redis_node["port"]) throw std::runtime_error("Config Error: Missing 'redis.port'");
    if (!redis_node["pool_size"]) throw std::runtime_error("Config Error: Missing 'redis.pool_size'. Please update config.yaml");

    // 取值
    const std::string host = redis_node["host"].as<std::string>();
    const int port = redis_node["port"].as<int>();
    const int pool_size = redis_node["pool_size"].as<int>();

    // 校验
    ValidateNotEmpty(host, "Redis Host");
    ValidatePort(port, "Redis Port");
    if (pool_size <= 0 || pool_size > 1000) {
        throw std::runtime_error(fmt::format("Config Error: Invalid Redis pool_size {}", pool_size));
    }

    // 赋值
    redis_config_.host = host;
    redis_config_.port = std::to_string(port);
    redis_config_.pool_size = pool_size;

    SPDLOG_INFO("Redis config loaded: {}:{}, PoolSize: {}", redis_config_.host, redis_config_.port, pool_size);
}

void AppConfig::ParseDbConfig(const YAML::Node& root_node) {
    // 一级节点检查
    if (!root_node["postgresql"]) throw std::runtime_error("Missing 'postgresql' section");
    const auto& db_node = root_node["postgresql"];
    // 二级节点检查
    if (!db_node["host"]) throw std::runtime_error("Config Error: Missing 'postgresql.host'");
    if (!db_node["port"]) throw std::runtime_error("Config Error: Missing 'postgresql.port'");
    if (!db_node["user"]) throw std::runtime_error("Config Error: Missing 'postgresql.user'");
    if (!db_node["password"]) throw std::runtime_error("Config Error: Missing 'postgresql.password'");
    if (!db_node["dbname"]) throw std::runtime_error("Config Error: Missing 'postgresql.dbname'");
    if (!db_node["pool_size"]) throw std::runtime_error("Config Error: Missing 'postgresql.pool_size'");

    // 取值
    const std::string host = db_node["host"].as<std::string>();
    const int port = db_node["port"].as<int>();
    const std::string user = db_node["user"].as<std::string>();
    const std::string pwd = db_node["password"].as<std::string>();
    const std::string dbname = db_node["dbname"].as<std::string>();
    const int pool_size = db_node["pool_size"].as<int>();

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
    // 一级节点检查
    if (!root_node["jwt"]) throw std::runtime_error("Missing 'jwt' section");
    const auto& node = root_node["jwt"];
    // 二级节点检查
    if (!node["secret_key"]) throw std::runtime_error("Config Error: Missing 'jwt.secret_key'");
    if (!node["issuer"]) throw std::runtime_error("Config Error: Missing 'jwt.issuer'");
    if (!node["expiration_seconds"]) throw std::runtime_error("Config Error: Missing 'jwt.expiration_seconds'");

    // 取值
    const std::string secret = node["secret_key"].as<std::string>();
    const std::string issuer = node["issuer"].as<std::string>();
    const int expire = node["expiration_seconds"].as<int>();

    // 校验
    ValidateNotEmpty(secret, "JWT Secret Key");
    ValidateNotEmpty(issuer, "JWT Issuer");
    if (expire <= 0) throw std::runtime_error("Config Error: JWT expiration_seconds must be positive");

    // 赋值
    jwt_config_ = {secret, issuer, expire};
    SPDLOG_INFO("JWT config loaded. Issuer: {}", issuer);
}

void AppConfig::ValidatePort(int port, const std::string& field_name) {
    if (port <= 0 || port > 65535) {
        throw std::runtime_error(
            fmt::format("Config Error: {} value '{}' is invalid. Must be between 1 and 65535", field_name, port)
        );
    }
}

void AppConfig::ValidateNotEmpty(const std::string& value, const std::string& field_name) {
    if (value.empty()) {
        throw std::runtime_error(fmt::format("Config Error: '{}' cannot be empty", field_name));
    }
}