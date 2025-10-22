// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/async_connection_pool.h"

using namespace common::db;


AsyncConnectionPool::AsyncConnectionPool(boost::asio::io_context &ioc, std::string conn_str, int pool_size)
    : ioc_(ioc),
      conn_str_(std::move(conn_str)),
      pool_size_(pool_size),
      // 初始化一个不会自动触发的定时器，用作异步条件变量
      waiter_(ioc, std::chrono::steady_clock::time_point::max()) {
    if (pool_size <= 0) {
        throw std::invalid_argument("Pool size must be positive.");
    }
}

boost::asio::awaitable<void> AsyncConnectionPool::Init() {
    spdlog::info("Initializing connection pool with size {}...", pool_size_);
    for (int i = 0; i < pool_size_; ++i) {
        auto conn = std::make_shared<PQConnection>(ioc_);
        co_await conn->async_connect(conn_str_);
        pool_.push_back(conn);
    }
    spdlog::info("Connection pool initialized successfully.");
}

boost::asio::awaitable<PooledConnectionPtr> AsyncConnectionPool::GetConnection() {
    std::unique_lock lock(mutex_);
    while (pool_.empty()) {
        lock.unlock();
        co_await waiter_.async_wait(boost::asio::use_awaitable);
        lock.lock();
    }
    const std::shared_ptr<PQConnection> conn_sh_ptr = pool_.front();
    pool_.pop_front();
    co_return PooledConnectionPtr(conn_sh_ptr.get(), ConnectionReleaser(conn_sh_ptr, shared_from_this()));
}

void AsyncConnectionPool::ReturnConnection(PQConnection *conn_ptr) {
    {
        std::lock_guard lock(mutex_);
        pool_.push_back(conn_ptr->shared_from_this());
    }
    waiter_.cancel_one();
}

// 实现 Deleter
inline void ConnectionReleaser::operator()(PQConnection *conn) const {
    // weak_ptr 若观察对象不存在会返回 shared_ptr 包裹一个nullptr
    if (auto pool_sh_ptr = pool.lock()) {
        pool_sh_ptr->ReturnConnection(conn);
    } else {
        // 如果连接池已经被销毁，就让 PQConnection 的 shared_ptr 自动管理内存
        spdlog::warn("Connection pool gone. Connection will be closed.");
    }
}
