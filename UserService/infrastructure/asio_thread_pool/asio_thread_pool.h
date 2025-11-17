// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <vector>
#include <thread>
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>
#include <optional>

namespace user_service::infrastructure {
    class AsioThreadPool {
    public:
        using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
        AsioThreadPool(const std::shared_ptr<boost::asio::io_context>& ioc);
        ~AsioThreadPool();

        // 启动和停止一定是幂等的
        void Run();
        void Stop();

    private:
        const std::shared_ptr<boost::asio::io_context> ioc_;
        std::optional<work_guard_type> work_guard_;
        std::vector<std::thread> asio_threads_;
    };
}