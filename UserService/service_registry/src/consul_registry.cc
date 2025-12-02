// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "service_registry/include/consul_registry.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

using namespace user_service::registry;
using json = nlohmann::json;

ConsulRegistry::ConsulRegistry(const ConsulConfig& consul_config): consul_(consul_config.GetAddress()) {
    SPDLOG_INFO("ConsulRegistry initialized. Target Agent: {}", consul_config.GetAddress());
}

ConsulRegistry::~ConsulRegistry() = default;

void ConsulRegistry::Register(const RegisterConfig &config) {
    try {
        // 生成唯一 Service ID
        // 格式: service_name-ip-port
        current_service_id_ = config.service_name + "-" + config.ip + "-" + std::to_string(config.port);

        // 构建 JSON Payload (参考 Consul Agent API)
        const json payload = {
            {"ID", current_service_id_},
            {"Name", config.service_name},
            {"Address", config.ip},
            {"Port", config.port},
            {"Tags", config.tags},
            {"Check", {
                // TCP 健康检查
                    {"TCP", std::format("{}:{}", config.ip, config.port)},
                    {"Interval", std::format("{}s", config.check_interval_seconds)},
                    {"Timeout", std::format("{}s", config.check_timeout_seconds)},
                    // 检查失败超过 60s，自动注销服务
                    {"DeregisterCriticalServiceAfter", "60s"}
            }}
        };
        // 注册接口: PUT /v1/agent/service/register
        consul_.put("/v1/agent/service/register", payload.dump());

        SPDLOG_INFO("Registered to Consul successfully. ServiceID: {}", current_service_id_);

    } catch (const std::exception &e) {
        SPDLOG_CRITICAL("Consul registration failed: {}", e.what());
        throw;
    }
}

std::vector<ServiceInstance> ConsulRegistry::Discover(const std::string &service_name) {
    std::vector<ServiceInstance> instances;
    try {
        // GET /v1/health/service/:service?passing=true
        std::string response_body = consul_.get("/v1/health/service/" + service_name + "?passing=true");

        if (response_body.empty()) {
            return instances;
        }

        // 解析返回的 JSON 数组
        const json j = json::parse(response_body);

        for (const auto& item : j) {
            ServiceInstance instance;

            // 提取 Service 信息
            const auto& s = item["Service"];
            instance.id = s["ID"].get<std::string>();
            instance.ip = s["Address"].get<std::string>();
            instance.port = s["Port"].get<int>();

            // 提取 Tags
            if (s.contains("Tags") && !s["Tags"].is_null()) {
                instance.tags = s["Tags"].get<std::vector<std::string>>();
            }

            // 提取 Metadata
            if (s.contains("Meta") && !s["Meta"].is_null()) {
                instance.metadata = s["Meta"].get<std::map<std::string, std::string>>();
            }

            instances.push_back(instance);
        }
    } catch (const std::exception &e) {
        SPDLOG_WARN("Consul discovery failed for {}: {}", service_name, e.what());
    }
    return instances;
}

void ConsulRegistry::Deregister() {
    if (current_service_id_.empty()) return;

    try {
        // 注销接口: PUT /v1/agent/service/deregister/<service_id>
        consul_.put("/v1/agent/service/deregister/" + current_service_id_, "");

        SPDLOG_INFO("Deregistered from Consul: {}", current_service_id_);
        current_service_id_.clear();
    } catch (const std::exception &e) {
        SPDLOG_WARN("Consul deregistration failed: {}", e.what());
    }
}
