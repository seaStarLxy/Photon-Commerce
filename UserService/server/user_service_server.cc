// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

//
// Created by root on 2025/10/19.
//

#include "user_service_server.h"
#include "call_data/include/register_call_data.h"
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

namespace user_service
{
    UserServiceServer::UserServiceServer(): ioc_(std::thread::hardware_concurrency()) {
        ioc_work_guard_.emplace(boost::asio::make_work_guard(ioc_));
    }
    UserServiceServer::~UserServiceServer() = default;

    void UserServiceServer::Run() {
        // 1. 启动 Asio 线程池 (用于业务逻辑)
        int asio_workers = std::thread::hardware_concurrency();
        SPDLOG_DEBUG("Starting {} Asio worker threads...", asio_workers);
        asio_threads_.reserve(asio_workers);
        for (int i = 0; i < asio_workers; ++i) {
            asio_threads_.emplace_back([this] {
                ioc_.run(); // 阻塞，直到 ioc 被 stop
            });
        }
        SPDLOG_DEBUG("Asio threads startup finished");

        // 2. 启动 gRPC 服务器
        std::string server_address("0.0.0.0:50051");
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        SPDLOG_DEBUG( "Server listening on {}", server_address);


        (new RegisterCallData(&service_, cq_.get(), ioc_))->Init();
        SPDLOG_DEBUG("Seeded 1st RegisterCallData.");


        // 启动 Worker 线程池
        int num_workers = std::thread::hardware_concurrency();
        SPDLOG_DEBUG("Starting {} worker threads...", num_workers);
        worker_threads_.reserve(num_workers);
        for (int i = 0; i < num_workers; ++i) {
            worker_threads_.emplace_back(&UserServiceServer::HandleRpc, this);
        }
        SPDLOG_DEBUG("Worker threads startup finished");

        // 阻塞主线程
        server_->Wait();
    }

    void UserServiceServer::Shutdown() {
        server_->Shutdown();
        cq_->Shutdown();
        for (auto& t : worker_threads_) {
            t.join();
        }
        ioc_work_guard_.reset();
        ioc_.stop();
        for (auto& t : asio_threads_) {
            t.join();
        }
        SPDLOG_DEBUG("Asio workers stopped.");
    }

    void UserServiceServer::HandleRpc() const {
        void* tag;  // tag 实际上是 ICallData*
        bool ok;

        // 循环：阻塞地从 CQ 中取事件
        while (cq_->Next(&tag, &ok)) {
            if (!ok) {
                // CQ 正在关闭
                break;
            }
            static_cast<ICallData*>(tag)->Proceed();
        }
    }

}