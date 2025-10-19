// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

//
// Created by root on 2025/10/19.
//

#include "user_service_server.h"

#include "call_data/include/register_call_data.h"

namespace UserService
{
    UserServiceServer::~UserServiceServer() = default;

    void UserServiceServer::Run() {
        std::string server_address("0.0.0.0:50051");

        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        new RegisterCallData(&service_, cq_.get());
        std::cout << "Seeded 1st RegisterCallData." << std::endl;

        // 启动 Worker 线程池
        int num_workers = std::thread::hardware_concurrency();
        std::cout << "Starting " << num_workers << " worker threads..." << std::endl;
        worker_threads_.reserve(num_workers);
        for (int i = 0; i < num_workers; ++i) {
            worker_threads_.emplace_back(&UserServiceServer::HandleRpc, this);
        }

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
        void* tag;  // tag 将是 ICallData*
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

} // UserService