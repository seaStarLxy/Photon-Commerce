// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

//
// Created by root on 2025/10/19.
//

#include "user_service_server.h"
#include "adapter/include/register_call_data.h"
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

namespace user_service::server
{
    UserServiceServer::UserServiceServer(const std::shared_ptr<service::IAuthService>& auth_service,
            const std::shared_ptr<service::IBasicUserService>& basic_service,
            const std::shared_ptr<boost::asio::io_context>& ioc):
    ioc_(ioc), auth_service_(auth_service), basic_service_(basic_service) {
        SPDLOG_DEBUG("haha");
        SPDLOG_DEBUG("UserServiceServer constructed. auth={}, basic={}, ioc={}",
             (void*)auth_service.get(), (void*)basic_service.get(), (void*)ioc.get());
    }
    UserServiceServer::~UserServiceServer() = default;

    void UserServiceServer::Run() {
        // 启动 gRPC 服务器
        std::string server_address("0.0.0.0:50051");
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        SPDLOG_DEBUG( "Server listening on {}", server_address);


        (new RegisterCallData(&service_, cq_.get(), *ioc_, basic_service_))->Init();
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