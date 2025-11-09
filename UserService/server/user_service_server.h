// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once

#include <UserService/v1/user_service.grpc.pb.h>
#include "service/interface/i_basic_user_service.h"
#include "service/interface/i_auth_service.h"
#include <thread>
#include <grpcpp/grpcpp.h>
#include <boost/asio/io_context.hpp>

namespace user_service::server
{
    class UserServiceServer
    {
    public:
        UserServiceServer(const std::shared_ptr<service::IAuthService>& auth_service,
            const std::shared_ptr<service::IBasicUserService>& basic_service,
            const std::shared_ptr<boost::asio::io_context>& ioc);
        ~UserServiceServer();
        void Run();
        void Shutdown();
    private:
        void HandleRpc() const;
        std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        v1::UserService::AsyncService service_;
        std::unique_ptr<grpc::Server> server_;
        std::vector<std::thread> worker_threads_;

        const std::shared_ptr<boost::asio::io_context> ioc_;
        std::shared_ptr<service::IAuthService> auth_service_;
        std::shared_ptr<service::IBasicUserService> basic_service_;
    };
}