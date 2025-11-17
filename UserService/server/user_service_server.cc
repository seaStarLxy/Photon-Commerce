// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "user_service_server.h"
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include "adapter/v2/call_data_manager/include/register_call_data_manager.h"
#include "adapter/v2/call_data/include/register_call_data.h"
#include "adapter/v2/call_data_manager/include/send_code_call_data_manager.h"
#include "adapter/v2/call_data/include/send_code_call_data.h"

// #include "adapter/v2/call_data/interface/call_data.hpp"

using namespace user_service::server;
using namespace user_service::adapter::v2;

UserServiceServer::UserServiceServer(const std::shared_ptr<service::IAuthService> &auth_service,
                                     const std::shared_ptr<service::IBasicUserService> &basic_service,
                                     const std::shared_ptr<boost::asio::io_context> &ioc) : ioc_(ioc),
    auth_business_service_(auth_service), basic_user_business_service_(basic_service) {
}

UserServiceServer::~UserServiceServer() = default;

void UserServiceServer::Run() {
    // 启动 gRPC 服务器
    std::string ipv4_address("0.0.0.0:50051");
    // std::string ipv6_address("[::]:50051");
    grpc::ServerBuilder builder;
    builder.AddListeningPort(ipv4_address, grpc::InsecureServerCredentials());
    // builder.AddListeningPort(ipv6_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&auth_grpc_service_);
    builder.RegisterService(&basic_user_grpc_service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    SPDLOG_DEBUG("Server listening on {}", ipv4_address);

    // 原始播种法
    // (new RegisterCallData(&service_, cq_.get(), *ioc_, basic_service_))->Init();
    SPDLOG_DEBUG("Seeded RegisterCallData.");
    RegisterCallDataManager register_manager = RegisterCallDataManager(100, &basic_user_grpc_service_, basic_user_business_service_.get(), ioc_, cq_.get());
    register_manager.Start();
    SendCodeCallDataManager send_code_manager = SendCodeCallDataManager(100, &auth_grpc_service_, auth_business_service_.get(), ioc_, cq_.get());
    send_code_manager.Start();


    // 启动 Worker 线程池
    unsigned num_workers = std::thread::hardware_concurrency();
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
    for (auto &t: worker_threads_) {
        t.join();
    }
}

void UserServiceServer::HandleRpc() const {
    void *tag; // tag 实际上是 ICallData*
    bool ok;

    // 循环：阻塞地从 CQ 中取事件
    while (cq_->Next(&tag, &ok)) {
        if (!ok) {
            // CQ 正在关闭
            break;
        }
        SPDLOG_DEBUG("Get one request");
        static_cast<ICallData*>(tag)->Proceed();
    }
}
