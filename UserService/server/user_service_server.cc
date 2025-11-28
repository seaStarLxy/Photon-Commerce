// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "user_service_server.h"
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include "adapter/v2/call_data_manager/include/register_call_data_manager.h"
#include "adapter/v2/call_data/include/register_call_data.h"
#include "adapter/v2/call_data_manager/include/send_code_call_data_manager.h"
#include "adapter/v2/call_data/include/send_code_call_data.h"
#include "adapter/v2/call_data_manager/include/login_by_password_call_data_manager.h"
#include "adapter/v2/call_data/include/login_by_password_call_data.h"
#include "adapter/v2/call_data_manager/include/login_by_code_call_data_manager.h"
#include "adapter/v2/call_data/include/login_by_code_call_data.h"
#include "adapter/v2/call_data_manager/include/get_user_info_call_data_manager.h"
#include "adapter/v2/call_data/include/get_user_info_call_data.h"


using namespace user_service::server;
using namespace user_service::adapter::v2;

UserServiceServer::UserServiceServer(const std::shared_ptr<service::IAuthService> &auth_service,
                                     const std::shared_ptr<service::IBasicUserService> &basic_service,
                                     const std::shared_ptr<util::IJwtUtil>& jwt_util,
                                     const std::shared_ptr<boost::asio::io_context> &ioc) : ioc_(ioc),
    auth_business_service_(auth_service), basic_user_business_service_(basic_service), jwt_util_(jwt_util) {
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
    // SPDLOG_DEBUG("Seeded RegisterCallData.");

    /* 模版播种法 */
    SPDLOG_DEBUG("Seeded Template CallData.");
    int call_data_num = 3000;
    // 注册
    RegisterCallDataManager register_manager = RegisterCallDataManager(call_data_num, &basic_user_grpc_service_, basic_user_business_service_.get(), jwt_util_.get(), ioc_, cq_.get());
    register_manager.Start();
    // 发送验证码
    SendCodeCallDataManager send_code_manager = SendCodeCallDataManager(call_data_num, &auth_grpc_service_, auth_business_service_.get(), jwt_util_.get(), ioc_, cq_.get());
    send_code_manager.Start();
    // 密码登录
    LoginByPasswordCallDataManager login_pw_manager(call_data_num, &auth_grpc_service_, auth_business_service_.get(), jwt_util_.get(), ioc_, cq_.get());
    login_pw_manager.Start();
    // 验证码登录
    LoginByCodeCallDataManager login_code_manager(call_data_num, &auth_grpc_service_, auth_business_service_.get(), jwt_util_.get(), ioc_, cq_.get());
    login_code_manager.Start();
    // 获取用户信息
    GetUserInfoCallDataManager get_user_info_manager(call_data_num, &basic_user_grpc_service_, basic_user_business_service_.get(), jwt_util_.get(), ioc_, cq_.get());
    get_user_info_manager.Start();


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
    SPDLOG_INFO("UserServiceServer shutting down...");

    // 停止 gRPC Server
    if (server_) {
        server_->Shutdown();
    }

    // 关闭 CQ
    if (cq_) {
        cq_->Shutdown();
    }

    // 安全回收 Worker 线程 (避免 std::terminate)
    for (auto &t: worker_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    worker_threads_.clear();

    SPDLOG_INFO("UserServiceServer shutdown finished.");
}

void UserServiceServer::HandleRpc() const {
    void *tag; // tag 实际上是 ICallData*
    bool ok;

    // 循环：阻塞地从 CQ 中取事件
    while (cq_->Next(&tag, &ok)) {
        SPDLOG_DEBUG("Get one request");
        static_cast<ICallData*>(tag)->Proceed(ok);
    }
}
