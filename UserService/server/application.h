// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <string>
#include <memory>
#include <boost/asio.hpp>

namespace user_service::infrastructure {
    class AsioThreadPool;
    class RedisClient;
    class AsyncConnectionPool;
}

namespace user_service::server {
    class UserServiceServer;
    class Application {
    public:
        explicit Application(std::string&& config_filepath);
        ~Application();

        // 不允许拷贝复制
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        // 运行整个程序
        void Run() const;

    private:
        std::string config_path_;
        const std::shared_ptr<boost::asio::io_context> ioc_;
        std::shared_ptr<infrastructure::RedisClient> redis_client_;
        std::shared_ptr<infrastructure::AsyncConnectionPool> db_pool_;
        std::unique_ptr<infrastructure::AsioThreadPool> thread_pool_;
        std::unique_ptr<UserServiceServer> server_;
    };
}