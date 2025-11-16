// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "server/user_service_server.h"
#include <spdlog/spdlog.h>
#include <boost/di.hpp>
#include "infrastructure/asio_thread_pool/asio_thread_pool.h"
#include "infrastructure/domain_implement/include/verification_code_repository.h"
#include "infrastructure/state_storage/redis/redis_client.h"
#include "service/include/auth_service.h"
#include "service/include/basic_user_service.h"
#include "utils/include/code_generator.h"
#include <boost/redis/src.hpp>


using namespace user_service::infrastructure;
using namespace user_service::service;
using namespace user_service::server;
using namespace user_service::util;
using namespace user_service::domain;


namespace di = boost::di;

void LogInit() {
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] [%s:%# (%!)] %v");
}

auto CreateInjector(const std::shared_ptr<boost::asio::io_context>& ioc_ptr, const RedisConfig& redis_config) {
    /*
     * bind<T> 要什么，传入T，可以自动解析构造函数中的 T T* T智能指针等等
     * to<U> 给什么，传入的U至少可以隐式转换成T。省略to则代表直接构造T类型
     * in 怎么给
     */
    // 注意这里的di内部会对类型解析，所以无法使用前向声明，必须把使用include头文件
    return di::make_injector(
        di::bind<boost::asio::io_context>().to(ioc_ptr),
        di::bind<RedisConfig>().to(redis_config),
        di::bind<RedisClient>().in(di::singleton),
        di::bind<ICodeGenerator>().to<CodeGenerator>().in(di::singleton),
        di::bind<IVerificationCodeRepository>().to<VerificationCodeRepository>().in(di::singleton),
        di::bind<IAuthService>().to<AuthService>().in(di::singleton),
        di::bind<IBasicUserService>().to<BasicUserService>().in(di::singleton)
    );
}

int main()
{
    // 初始化日志服务
    LogInit();

    try {
        // 构建依赖注入控制器
        SPDLOG_INFO("starting create injector");
        auto ioc_ptr = std::make_shared<boost::asio::io_context>();

        const RedisConfig app_redis_config = {"redis-cache", "6379"};
        const auto injector = CreateInjector(ioc_ptr, app_redis_config);

        // 构建 asio 线程池
        SPDLOG_INFO("starting create asio thread pool");
        const auto asio_thread_pool = injector.create<std::unique_ptr<AsioThreadPool>>();
        SPDLOG_INFO("asio thread pool created. Starting to run...");
        // 启动 asio 线程池
        asio_thread_pool->Run();

        SPDLOG_INFO("Starting RedisClient...");
        // RedisClient 是单例，用 & 获取它的引用
        auto& redis_client = injector.create<RedisClient&>();
        redis_client.Run();
        SPDLOG_INFO("RedisClient started.");

        // 构建微服务启动器
        SPDLOG_INFO("starting create server");
        const auto server = injector.create<std::unique_ptr<UserServiceServer>>();
        // 启动微服务
        SPDLOG_INFO("UserServiceServer created. Starting server...");
        server->Run();

    } catch (const std::exception& e) {
        SPDLOG_ERROR("Application failed to run: {}", e.what());
    }
    SPDLOG_INFO("Application shutting down...");
    return 0;
}
