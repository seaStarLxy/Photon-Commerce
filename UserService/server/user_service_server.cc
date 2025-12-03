// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "user_service_server.h"
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include "adapter/v2/call_data/include/register_call_data.h"
#include "adapter/v2/call_data/include/send_code_call_data.h"
#include "adapter/v2/call_data/include/login_by_password_call_data.h"
#include "adapter/v2/call_data/include/login_by_code_call_data.h"
#include "adapter/v2/call_data/include/get_user_info_call_data.h"

#include "adapter/v2/call_data_manager/include/register_call_data_manager.h"
#include "adapter/v2/call_data_manager/include/send_code_call_data_manager.h"
#include "adapter/v2/call_data_manager/include/login_by_password_call_data_manager.h"
#include "adapter/v2/call_data_manager/include/login_by_code_call_data_manager.h"
#include "adapter/v2/call_data_manager/include/get_user_info_call_data_manager.h"

using namespace user_service::server;
using namespace user_service::adapter::v2;

UserServiceServer::UserServiceServer(const ServerConfig &server_config,
                                     const RpcLimitsConfig &rpc_limits,
                                     const std::shared_ptr<registry::ServiceRegistry> &registry,
                                     const std::shared_ptr<service::IAuthService> &auth_service,
                                     const std::shared_ptr<service::IBasicUserService> &basic_service,
                                     const std::shared_ptr<util::IJwtUtil> &jwt_util,
                                     const std::shared_ptr<boost::asio::io_context> &ioc) : server_config_(
        server_config), rpc_limits_(rpc_limits), registry_(registry), ioc_(ioc),
    auth_business_service_(auth_service), basic_user_business_service_(basic_service), jwt_util_(jwt_util) {
}

UserServiceServer::~UserServiceServer() = default;

void UserServiceServer::Run() {
    // 启动 gRPC 服务器
    StartServer();

    // 注册服务到 consul
    // registry_->Register(server_config_.register_info);

    // 播种 CallData
    SeedCallData();

    // 启动监听 Worker 线程池
    StartListeningThread();

    // 阻塞主线程
    server_->Wait();
}

void UserServiceServer::Shutdown() {
    SPDLOG_INFO("UserServiceServer shutting down...");

    // 停止 gRPC Server
    if (server_) {
        server_->Shutdown();
    }

    // 关闭 CQ
    if (cq_) {
        cq_->Shutdown();
    }

    if (!worker_threads_.empty()) {
        // 安全回收 Worker 线程 (避免 std::terminate)
        for (auto &t: worker_threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
        worker_threads_.clear();
    } else {
        /* 调试期间：服务发现 consul 未连接成功抛出异常，程序进入析构。
         * 此时导致初始化过程完成一半，也就是 server 起来了，cq 起来了，但是没有初始化监听线程
         * 程序正常析构，cq中包含server->Shutdown产生的事件，需要取出才可析构，否则会抛异常 */
        if (cq_) {
            SPDLOG_WARN("Worker threads not started, draining CQ manually...");
            void* tag;
            bool ok;
            // 泄洪操作，把残留事件排空
            while (cq_->Next(&tag, &ok)) {
            }
        }

    }

    SPDLOG_INFO("UserServiceServer shutdown finished.");
}

void UserServiceServer::HandleRpc() const {
    void *tag; // tag 实际上是 ICallData*
    bool ok;

    // 循环：阻塞地从 CQ 中取事件
    while (cq_->Next(&tag, &ok)) {
        SPDLOG_DEBUG("Get one request");
        static_cast<ICallData *>(tag)->Proceed(ok);
    }
}

void UserServiceServer::StartServer() {
    std::string server_address = std::format("{}:{}", server_config_.bind_ip, server_config_.port);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&auth_grpc_service_);
    builder.RegisterService(&basic_user_grpc_service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    SPDLOG_DEBUG("Server listening on {}", server_address);
}

void UserServiceServer::StartListeningThread() {
    SPDLOG_DEBUG("Starting {} worker threads...", server_config_.worker_threads);
    worker_threads_.reserve(server_config_.worker_threads);
    for (int i = 0; i < server_config_.worker_threads; ++i) {
        worker_threads_.emplace_back(&UserServiceServer::HandleRpc, this);
    }
    SPDLOG_DEBUG("Worker threads startup finished");
}

void UserServiceServer::SeedCallData() {
    // 原始播种法
    // (new RegisterCallData(&service_, cq_.get(), *ioc_, basic_service_))->Init();
    // SPDLOG_DEBUG("Seeded RegisterCallData.");

    /* 模版播种法 */
    SPDLOG_DEBUG("Seeded Template CallData.");
    // 注册
    register_manager_ = std::make_unique<RegisterCallDataManager>(
        rpc_limits_.register_num,
        &basic_user_grpc_service_,
        basic_user_business_service_.get(),
        jwt_util_.get(), ioc_, cq_.get());
    register_manager_->Start();

    // 发送验证码
    send_code_manager_ = std::make_unique<SendCodeCallDataManager>(
        rpc_limits_.send_code_num,
        &auth_grpc_service_,
        auth_business_service_.get(), jwt_util_.get(),
        ioc_, cq_.get());
    send_code_manager_->Start();

    // 密码登录
    login_pw_manager_ = std::make_unique<LoginByPasswordCallDataManager>(
        rpc_limits_.login_pw_num,
        &auth_grpc_service_, auth_business_service_.get(),
        jwt_util_.get(), ioc_, cq_.get());
    login_pw_manager_->Start();

    // 验证码登录
    login_code_manager_ = std::make_unique<LoginByCodeCallDataManager>(
        rpc_limits_.login_code_num,
        &auth_grpc_service_, auth_business_service_.get(),
        jwt_util_.get(), ioc_, cq_.get());
    login_code_manager_->Start();

    // 获取用户信息
    get_user_info_manager_ = std::make_unique<GetUserInfoCallDataManager>(
        rpc_limits_.get_user_info_num,
        &basic_user_grpc_service_,
        basic_user_business_service_.get(), jwt_util_.get(), ioc_,
        cq_.get());
    get_user_info_manager_->Start();
}