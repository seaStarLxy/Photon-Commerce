// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "pq_connection.h"
#include <deque>
#include <mutex>
#include <memory>
#include <utility>
#include <spdlog/spdlog.h>

namespace common::db {
    class AsyncConnectionPool;
    // Deleter 的工作不是 delete 连接，而是将其归还给连接池
    struct ConnectionReleaser {
        explicit ConnectionReleaser(const std::shared_ptr<PQConnection> &conn,
                                    const std::shared_ptr<AsyncConnectionPool> &p) : conn(conn), pool(p) {
        }
        // 实现放后面，因为它需要完整的 Pool 定义
        void operator()(PQConnection *conn) const;
        // 持有 PQConnection的shared_ptr 保证不被自动销毁
        std::shared_ptr<PQConnection> conn;
        // 使用 weak_ptr 防止循环引用，并且是线程安全的
        std::weak_ptr<AsyncConnectionPool> pool;
    };

    // 封装单个数据库连接，并添加 deleter
    using PooledConnectionPtr = std::unique_ptr<PQConnection, ConnectionReleaser>;

    class AsyncConnectionPool : public std::enable_shared_from_this<AsyncConnectionPool> {
    public:
        AsyncConnectionPool(boost::asio::io_context &ioc, std::string conn_str, int pool_size);

        boost::asio::awaitable<void> Init();

        boost::asio::awaitable<PooledConnectionPtr> GetConnection();

    private:
        friend struct ConnectionReleaser;

        void ReturnConnection(PQConnection *conn_ptr);

        boost::asio::io_context &ioc_;
        std::string conn_str_;
        int pool_size_;
        std::mutex mutex_;
        std::deque<std::shared_ptr<PQConnection> > pool_;
        boost::asio::steady_timer waiter_;
    };
}
