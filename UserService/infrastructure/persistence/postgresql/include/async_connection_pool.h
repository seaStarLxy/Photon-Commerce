// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "pq_connection.h"
#include <deque>
#include <mutex>
#include <memory>
#include <boost/asio/experimental/channel.hpp>
#include <spdlog/spdlog.h>

namespace user_service::infrastructure {
    // 配置文件
    struct DbPoolConfig {
        std::string conn_str;
        int pool_size;
    };
    class AsyncConnectionPool;
    // Deleter 的工作不是 delete 连接，而是将其归还给连接池
    struct ConnectionReleaser {
        explicit ConnectionReleaser(const std::shared_ptr<PQConnection> &conn,
                                    const std::shared_ptr<AsyncConnectionPool> &p) : conn_sh_ptr(conn), pool(p) {
        }
        // 实现放后面，因为它需要完整的 Pool 定义，参数需要一定保证是罗指针裸指针
        void operator()(PQConnection *conn) const;
        // 持有 PQConnection的shared_ptr 保证不被自动销毁
        std::shared_ptr<PQConnection> conn_sh_ptr;
        // 使用 weak_ptr 防止循环引用，并且是线程安全的
        std::weak_ptr<AsyncConnectionPool> pool;
    };

    // 封装池化数据库连接，并添加 deleter
    using PooledConnection = std::unique_ptr<PQConnection, ConnectionReleaser>;

    class AsyncConnectionPool : public std::enable_shared_from_this<AsyncConnectionPool> {
    public:
        AsyncConnectionPool(const std::shared_ptr<boost::asio::io_context>& ioc, const DbPoolConfig& db_pool_config);

        boost::asio::awaitable<void> Init();

        boost::asio::awaitable<PooledConnection> GetConnection();

    private:
        friend struct ConnectionReleaser;

        void ReturnConnection(const std::shared_ptr<PQConnection>& conn_sh_ptr);

        const std::shared_ptr<boost::asio::io_context> ioc_;
        const std::string conn_str_;
        const int pool_size_;
        std::mutex mutex_;
        // 注意 PQConnection 不可复制
        std::deque<std::shared_ptr<PQConnection> > pool_;
        /* 原计划使用steady_timer作为条件变量，但是在获取连接部分，
         * unlock后和协程挂起前的缝隙，如果发生了notify_one操作会导致这个唤醒丢失，
         * 所以这个位置需要一个原子操作来实现unlock和协程挂起，按照stdz
         */
        // boost::asio::steady_timer waiter_;
        boost::asio::experimental::channel<void(boost::system::error_code)> signal_channel_;
    };
}
