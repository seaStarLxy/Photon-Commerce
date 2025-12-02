// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once

#include "service_registry/interface/service_registry.h"
#include <ppconsul/consul.h>

namespace user_service::registry {

    // consul 配置
    struct ConsulConfig {
        std::string ip;     // Consul Agent 的 IP
        int port;           // Consul Agent 的 HTTP 端口

        // 获取完整的 "ip:port" 字符串
        [[nodiscard]] std::string GetAddress() const {
            return ip + ":" + std::to_string(port);
        }
    };

    class ConsulRegistry : public ServiceRegistry {
    public:
        explicit ConsulRegistry(const ConsulConfig& consul_config);

        ~ConsulRegistry() override;

        void Register(const RegisterConfig &config) override;

        std::vector<ServiceInstance> Discover(const std::string &service_name) override;

        void Deregister() override;

    private:
        ppconsul::Consul consul_;
        std::string current_service_id_; // 保存 ID 用于注销
    };
}
