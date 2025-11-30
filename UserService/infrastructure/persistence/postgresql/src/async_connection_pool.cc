// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "../include/async_connection_pool.h"

using namespace user_service::infrastructure;


AsyncConnectionPool::AsyncConnectionPool(const std::shared_ptr<boost::asio::io_context>& ioc, const DbPoolConfig& db_pool_config)
    : ioc_(ioc),
      conn_str_(db_pool_config.conn_str),
      pool_size_(db_pool_config.pool_size),
      strand_(boost::asio::make_strand(*ioc)),
      waiters_channel_(strand_, 0)
{
    if (pool_size_ <= 0) throw std::invalid_argument("Pool size must be positive.");
}

boost::asio::awaitable<void> AsyncConnectionPool::Init() {
    SPDLOG_DEBUG("Initializing connection pool with size {}...", pool_size_);
    // Init 是串行调用的，不需要锁保护
    for (int i = 0; i < pool_size_; ++i) {
        auto conn = std::make_shared<PQConnection>(*ioc_);
        co_await conn->AsyncConnect(conn_str_);
        pool_.push_back(conn);
    }
    SPDLOG_DEBUG("Connection pool initialized successfully.");
}

boost::asio::awaitable<PooledConnection> AsyncConnectionPool::GetConnection() {

    // post 到串行区执行
    co_await boost::asio::post(strand_, boost::asio::use_awaitable);

    std::shared_ptr<PQConnection> conn;

    if (!pool_.empty()) {
        conn = pool_.front();
        pool_.pop_front();
    } else {
        // 把当前协程挂起并放入 Channel 的内部队列
        conn = co_await waiters_channel_.async_receive(boost::asio::use_awaitable);
    }

    // 无论是从池子拿的，还是别人用完了的，conn 都有值了
    co_return PooledConnection(conn.get(), ConnectionReleaser(conn, shared_from_this()));
}

void AsyncConnectionPool::ReturnConnection(const std::shared_ptr<PQConnection>& conn_sh_ptr) {
    // 任务提交到 strand 串行区执行
    boost::asio::post(strand_, [this, conn = conn_sh_ptr]() mutable {
        // 尝试直接发送给等待者
        // try_send 检查 Channel 内部队列是否有挂起的协程
        // 如果有，返回 true 并直接把 conn 塞给他唤醒；如果没有，返回 false
        const bool given_to_waiter = waiters_channel_.try_send(boost::system::error_code{}, conn);

        if (!given_to_waiter) {
            // 没有挂起等待的协程了，放回池子
            pool_.push_back(conn);
        }
    });
}

void ConnectionReleaser::operator()(PQConnection *conn) const {
    if (const auto pool_sh_ptr = pool.lock()) {
        pool_sh_ptr->ReturnConnection(conn_sh_ptr);
    } else {
        spdlog::warn("Connection pool gone. Connection will be closed.");
    }
}