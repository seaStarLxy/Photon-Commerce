// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once

#include <UserService/v1/user_service.grpc.pb.h>
#include "service/interface/i_basic_user_service.h"
#include "service/interface/i_auth_service.h"
#include "utils/interface/i_jwt_util.h"
#include "service_registry/interface/service_registry.h"
#include <thread>
#include <grpcpp/grpcpp.h>
#include <boost/asio/io_context.hpp>

namespace user_service::server {
    // 配置 server
    struct ServerConfig {
        // 部署信息
        std::string bind_ip;
        int port;
        int listen_threads;
        int worker_threads;
        // 服务注册信息
        registry::RegisterConfig register_info;
    };

    // RPC 限制配置
    struct RpcLimitsConfig {
        int register_num;
        int send_code_num;
        int login_pw_num;
        int login_code_num;
        int get_user_info_num;
    };

    class UserServiceServer {
    public:
        UserServiceServer(
            const ServerConfig &server_config,
            const RpcLimitsConfig &rpc_limits,
            const std::shared_ptr<registry::ServiceRegistry> &registry,
            const std::shared_ptr<service::IAuthService> &auth_service,
            const std::shared_ptr<service::IBasicUserService> &basic_service,
            const std::shared_ptr<util::IJwtUtil> &jwt_util,
            const std::shared_ptr<boost::asio::io_context> &ioc);

        ~UserServiceServer();

        void Run();

        void Shutdown();

    private:
        void HandleRpc() const;


        std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        proto::v1::AuthService::AsyncService auth_grpc_service_;
        proto::v1::UserService::AsyncService basic_user_grpc_service_;
        std::unique_ptr<grpc::Server> server_;
        std::vector<std::thread> worker_threads_;

        ServerConfig server_config_;
        RpcLimitsConfig rpc_limits_;
        const std::shared_ptr<registry::ServiceRegistry> registry_;
        const std::shared_ptr<boost::asio::io_context> ioc_;
        const std::shared_ptr<service::IAuthService> auth_business_service_;
        const std::shared_ptr<service::IBasicUserService> basic_user_business_service_;
        const std::shared_ptr<util::IJwtUtil> jwt_util_;
    };
}
