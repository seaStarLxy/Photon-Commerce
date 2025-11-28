// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "asio_thread_pool.h"
#include <spdlog/spdlog.h>

using namespace user_service::infrastructure;

AsioThreadPool::AsioThreadPool(const std::shared_ptr<boost::asio::io_context>& ioc): ioc_(ioc) {
    SPDLOG_DEBUG("Execute AsioThreadPool Constructor");
}

AsioThreadPool::~AsioThreadPool() {
    // 这里保证RAII, 即使不手动Stop, 也保证释放时Stop
    Stop();
}

void AsioThreadPool::Run() {
    // 保证幂等
    if (work_guard_) {
        SPDLOG_WARN("AsioThreadPool is already running.");
        return;
    }

    // 创建 work_guard，防止 ioc->run() 没有任务就立即退出
    work_guard_.emplace(boost::asio::make_work_guard(*ioc_));

    unsigned int thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0) thread_count = 2; // 保底 2 个线程
    // unsigned int thread_count = 1;
    asio_threads_.reserve(thread_count);
    for (unsigned i = 0; i < thread_count; ++i) {
        asio_threads_.emplace_back([ioc = ioc_]() {
            SPDLOG_TRACE("start run");
            ioc->run();
            SPDLOG_TRACE("finish run");
        });
    }
    SPDLOG_INFO("Asio thread pool started with {} threads.", thread_count);
}

// 停止线程池
void AsioThreadPool::Stop() {
    // 保证幂等
    if (!work_guard_) {
        return;
    }
    SPDLOG_DEBUG("Stopping Asio thread pool...");

    // 释放 work_guard，允许 ioc->run() 退出
    work_guard_.reset();
    if (ioc_ && !ioc_->stopped()) {
        ioc_->stop();
    }
    // 等待所有线程退出
    for (auto& t : asio_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    asio_threads_.clear();
    SPDLOG_INFO("Asio thread pool stopped.");
}