// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace user_service::registry {

    struct RegisterConfig {
        std::string service_name;       // 服务名 "xxx-service"
        std::string ip;                 // 外部可访问 IP "172.31.x.x"
        int port;                       // 端口
        std::vector<std::string> tags;  // 标签 {"v1", "stable"}

        // 健康检查要求
        int check_interval_seconds = 10;
        int check_timeout_seconds = 2;
    };

    // 用于服务发现
    struct ServiceInstance {
        std::string id;           // 唯一ID
        std::string ip;           // 目标IP
        int port;                 // 目标端口

        std::map<std::string, std::string> metadata; // 元数据 (权重、区域等)
        std::vector<std::string> tags;

        // 获取完整的 "ip:port" 字符串
        std::string GetAddress() const {
            return ip + ":" + std::to_string(port);
        }
    };

    class ServiceRegistry {
    public:
        virtual ~ServiceRegistry() = default;

        // 服务注册
        virtual void Register(const RegisterConfig& config) = 0;

        // 服务发现
        virtual std::vector<ServiceInstance> Discover(const std::string& service_name) = 0;

        // 服务注销（自己）
        virtual void Deregister() = 0;
    };

}