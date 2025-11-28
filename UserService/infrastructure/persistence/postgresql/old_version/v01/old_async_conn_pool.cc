// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "infrastructure/persistence/postgresql/old_version/v01/old_async_conn_pool.h"
#include <spdlog/spdlog.h>

using namespace user_service::infrastructure;


AsyncConnectionPool::AsyncConnectionPool(const std::shared_ptr<boost::asio::io_context>& ioc, const DbPoolConfig& db_pool_config)
    : ioc_(ioc),
      conn_str_(db_pool_config.conn_str),
      pool_size_(db_pool_config.pool_size),
      signal_channel_(*ioc, (std::numeric_limits<std::size_t>::max)()) {
    SPDLOG_DEBUG("Execute AsyncConnectionPool Constructor");
    if (pool_size_ <= 0) {
        throw std::invalid_argument("Pool size must be positive.");
    }
}

boost::asio::awaitable<void> AsyncConnectionPool::Init() {
    spdlog::info("Initializing connection pool with size {}...", pool_size_);
    for (int i = 0; i < pool_size_; ++i) {
        auto conn = std::make_shared<PQConnection>(*ioc_);
        co_await conn->AsyncConnect(conn_str_);
        {
            std::lock_guard lock(mutex_);
            pool_.push_back(conn);
        }
    }
    spdlog::info("Connection pool initialized successfully.");
}

boost::asio::awaitable<PooledConnection> AsyncConnectionPool::GetConnection() {
    std::unique_lock lock(mutex_);
    while (pool_.empty()) {
        lock.unlock();
        // co_await waiter_.async_wait(boost::asio::use_awaitable);
        (void) co_await signal_channel_.async_receive(boost::asio::use_awaitable);
        lock.lock();
    }
    const std::shared_ptr<PQConnection> conn_sh_ptr = pool_.front();
    pool_.pop_front();
    co_return PooledConnection(conn_sh_ptr.get(), ConnectionReleaser(conn_sh_ptr, shared_from_this()));
}

void AsyncConnectionPool::ReturnConnection(const std::shared_ptr<PQConnection>& conn_sh_ptr) {
    {
        // 没有解锁操作，纯靠RAII，但unique_lock可以自己解锁
        std::lock_guard lock(mutex_);
        pool_.push_back(conn_sh_ptr);
    }
    // 条件变量唤醒
    // waiter_.cancel_one();
    // 发信号唤醒
    signal_channel_.try_send(boost::system::error_code{});

}

// 实现 Deleter
void ConnectionReleaser::operator()(PQConnection *conn) const {
    // weak_ptr 若观察对象不存在会返回 shared_ptr 包裹一个nullptr
    if (auto pool_sh_ptr = pool.lock()) {
        pool_sh_ptr->ReturnConnection(conn_sh_ptr);
    } else {
        // 如果连接池已经被销毁，就让 PQConnection 的 shared_ptr 自动管理内存
        spdlog::warn("Connection pool gone. Connection will be closed.");
    }
}
