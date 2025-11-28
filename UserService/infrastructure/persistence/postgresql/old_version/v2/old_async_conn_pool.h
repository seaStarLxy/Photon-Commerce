// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "infrastructure/persistence/postgresql/include/pq_connection.h"
#include <deque>
#include <memory>
#include <coroutine>
#include <boost/asio/strand.hpp>

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
                                    const std::shared_ptr<AsyncConnectionPool> &p) : conn_sh_ptr(conn), pool(p) {}
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

        // 获取连接 (Strand 串行执行)
        boost::asio::awaitable<PooledConnection> GetConnection();

    private:
        friend struct ConnectionReleaser;
        // 归还连接 (Strand 串行执行)
        void ReturnConnection(const std::shared_ptr<PQConnection>& conn_sh_ptr);


        // 用于维护被挂起的协程帧，以及需要被分配的连接
        struct Waiter {
            // 协程帧
            std::coroutine_handle<> handle;
            // 挂起时为空，恢复之前会需要填入连接
            std::shared_ptr<PQConnection> assigned_conn;
        };

        /*
         * 类中结构体声明：
         * 用于自定义可等待对象，包括：
         * 挂起状态检查: await_ready
         * 如何挂起: await_suspend
         * 如何恢复: await_resume
         * 注意：一定要在 strand 串行区挂起，内部
         */
        struct WaitForConnectionAwaiter;

        const std::shared_ptr<boost::asio::io_context> ioc_;
        const std::string conn_str_;
        const int pool_size_;

        // Strand 调度器，用于串行执行：获取连接、归还连接
        boost::asio::strand<boost::asio::io_context::executor_type> strand_;

        // 空闲连接队列
        std::deque<std::shared_ptr<PQConnection>> pool_;

        // 维护等待者队列
        // 存储指向协程帧内部 Waiter 对象的指针
        std::deque<Waiter*> waiters_;

    };
}