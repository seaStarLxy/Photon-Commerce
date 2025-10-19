// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once

#include <thread>
#include <UserService/v1/user_service.grpc.pb.h>
#include <grpcpp/grpcpp.h>

namespace UserService
{
    class UserServiceServer
    {
    public:
        ~UserServiceServer();
        void Run();
        void Shutdown();
    private:
        void HandleRpc() const;
        std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        v1::UserService::AsyncService service_;
        std::unique_ptr<grpc::Server> server_;
        std::vector<std::thread> worker_threads_;
    };
} // UserService