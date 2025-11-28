// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "infrastructure/persistence/postgresql/old_version/v2/old_async_conn_pool.h"
#include <spdlog/spdlog.h>

using namespace user_service::infrastructure;

// ==========================================
// 协程核心：自定义 Awaiter 实现
// ==========================================
struct AsyncConnectionPool::WaitForConnectionAwaiter {
    AsyncConnectionPool* pool_;
    Waiter waiter_;

    explicit WaitForConnectionAwaiter(AsyncConnectionPool* pool)
        : pool_(pool), waiter_(nullptr, nullptr) {}

    // 连接池为空时创建 Awaiter，一定需要挂起，直接返回 false
    bool await_ready() const noexcept { return false; }

    // 挂起逻辑（串行区无线程安全问题）
    void await_suspend(std::coroutine_handle<> h) {
        waiter_.handle = h;
        // 加入到等待队列
        pool_->waiters_.push_back(&waiter_);

        // 函数返回后，协程成功挂起，Strand 去执行别的任务
    }

    // 恢复时的逻辑
    std::shared_ptr<PQConnection> await_resume() const noexcept {
        // ReturnConnection 把连接填进来后，协程才被唤醒
        return waiter_.assigned_conn;
    }
};

// ==========================================
//  AsyncConnectionPool 实现
// ==========================================

AsyncConnectionPool::AsyncConnectionPool(const std::shared_ptr<boost::asio::io_context>& ioc, const DbPoolConfig& db_pool_config)
    : ioc_(ioc),
      conn_str_(db_pool_config.conn_str),
      pool_size_(db_pool_config.pool_size),
      strand_(boost::asio::make_strand(*ioc))   // 初始化 Strand
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
        // 挂起排队
        conn = co_await WaitForConnectionAwaiter{this};
    }

    // 无论是从池子拿的，还是别人用完了的，conn 都有值了
    co_return PooledConnection(conn.get(), ConnectionReleaser(conn, shared_from_this()));
}

void AsyncConnectionPool::ReturnConnection(const std::shared_ptr<PQConnection>& conn_sh_ptr) {
    // 任务提交到 strand 串行区执行
    boost::asio::post(strand_, [this, conn = conn_sh_ptr]() mutable {
        if (!waiters_.empty()) {
            // 从 waiter 池子中取出一个 waiter*
            Waiter* waiter = waiters_.front();
            waiters_.pop_front();

            // 把连接交给 waiter
            waiter->assigned_conn = conn;

            // 唤醒 waiter (告诉他有连接可以用了)
            waiter->handle.resume();
        } else {
            // 没有挂起等待的写成了，放回池子
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