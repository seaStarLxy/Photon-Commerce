// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once

#include <thread>
#include <UserService/v1/user_service.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <boost/asio/io_context.hpp>

namespace user_service
{
    class UserServiceServer
    {
    public:
        UserServiceServer();
        ~UserServiceServer();
        void Run();
        void Shutdown();
    private:
        void HandleRpc() const;
        std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        v1::UserService::AsyncService service_;
        std::unique_ptr<grpc::Server> server_;
        std::vector<std::thread> worker_threads_;

        using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
        std::optional<work_guard_type> ioc_work_guard_;
        boost::asio::io_context ioc_; // 提升为成员变量
        std::vector<std::thread> asio_threads_; // Asio 线程池
    };
} // UserService