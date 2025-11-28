// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "server/application.h"
#include <spdlog/spdlog.h>
#include <boost/di.hpp>
#include <utility>
#include "server/user_service_server.h"

#include "service/include/auth_service.h"
#include "service/include/basic_user_service.h"

#include "infrastructure/asio_thread_pool/asio_thread_pool.h"
#include "infrastructure/state_storage/redis_dao/redis_client.h"
#include "infrastructure/persistence/postgresql/include/async_connection_pool.h"
#include "infrastructure/persistence/dao/user_dao.h"
#include "infrastructure/domain_implement/include/verification_code_repository.h"
#include "infrastructure/domain_implement/include/user_repository.h"

#include "utils/include/verification_code_generator.h"
#include "utils/include/id_generator.h"
#include "utils/include/security_util.h"
#include "utils/include/jwt_util.h"

#include "config/app_config.h"

using namespace user_service::infrastructure;
using namespace user_service::service;
using namespace user_service::server;
using namespace user_service::util;
using namespace user_service::domain;
using namespace user_service::config;

namespace di = boost::di;

Application::Application(std::string&& config_filepath): config_path_(std::move(config_filepath)),
    ioc_(std::make_shared<boost::asio::io_context>()) {

    SPDLOG_INFO("Application constructing...");

    // 加载配置
    const auto app_config = AppConfig(config_path_);
    const auto redis_config = app_config.GetRedisConfig();
    const auto db_pool_config = app_config.GetDBPoolConfig();
    const auto jwt_config = app_config.GetJwtConfig();

    /*
     * bind<T> 要什么，传入T，可以自动解析构造函数中的 T T* T智能指针等等
     * to<U> 给什么，传入的U至少可以隐式转换成T。省略to则代表直接构造T类型
     * in 怎么给
     */
    // 注意这里的di内部会对类型解析，所以无法使用前向声明，必须使用include头文件（现在在源文件其实不用管）
    // 构造依赖注入器
    /* ps: ioc一定要传入const指针，否则di内部会创建其他实例 */
    const auto injector = di::make_injector(
        di::bind<boost::asio::io_context>().to(ioc_),
        di::bind<DbPoolConfig>().to(db_pool_config),
        di::bind<AsyncConnectionPool>().in(di::singleton),
        di::bind<UserDao>().in(di::singleton),
        di::bind<RedisConfig>().to(redis_config),
        di::bind<RedisClient>().in(di::singleton),
        di::bind<IVerificationCodeGenerator>().to<CodeGenerator>().in(di::singleton),
        di::bind<IIDGenerator>().to<IdGenerator>().in(di::singleton),
        di::bind<ISecurityUtil>().to<SecurityUtil>().in(di::singleton),
        di::bind<JwtConfig>().to(jwt_config),
        di::bind<IJwtUtil>().to<JwtUtil>().in(di::singleton),
        di::bind<IVerificationCodeRepository>().to<VerificationCodeRepository>().in(di::singleton),
        di::bind<IUserRepository>().to<UserRepository>().in(di::singleton),
        di::bind<IAuthService>().to<AuthService>().in(di::singleton),
        di::bind<IBasicUserService>().to<BasicUserService>().in(di::singleton)
    );
    // 获取核心资源（后面需要初始化）
    redis_client_ = injector.create<std::shared_ptr<RedisClient>>();
    db_pool_ = injector.create<std::shared_ptr<AsyncConnectionPool>>();
    // 创建 Server 和 ThreadPool
    thread_pool_ = injector.create<std::unique_ptr<AsioThreadPool>>();
    server_ = injector.create<std::unique_ptr<UserServiceServer>>();
    SPDLOG_INFO("Application constructed.");
}

Application::~Application() {
    thread_pool_->Stop();
}

void Application::Run() const {
    try {
        // 启动线程池
        SPDLOG_INFO("Application: Starting Thread Pool...");
        SPDLOG_INFO("DEBUG CHECK: Application ioc address: {}", fmt::ptr(ioc_.get()));
        thread_pool_->Run();

        SPDLOG_INFO("Application: Waiting for infrastructure init...");
        // 同步等待，初始化完成后才能开始监听
        std::future<void> init_future = boost::asio::co_spawn(*ioc_, [this]() -> boost::asio::awaitable<void> {
            try {
                // Redis 初始化
                co_await redis_client_->Init();
                SPDLOG_INFO("Redis connected.");
                // 数据库连接池 初始化
                co_await db_pool_->Init();
                SPDLOG_INFO("Database connected.");
            } catch (const std::exception& e) {
                SPDLOG_CRITICAL("Infrastructure init failed: {}", e.what());
                throw;
            }
        }, boost::asio::use_future);
        init_future.get();

        // 启动 Server
        SPDLOG_INFO("Application: Starting gRPC Server...");
        server_->Run();
    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("Application runtime error: {}", e.what());
        throw;
    }
}