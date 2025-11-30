// Copyright (c) 2025 seaStarLxy.
 // Licensed under the MIT License.

 #pragma once
 #include "pq_connection.h"
 #include <deque>
 #include <memory>
 #include <boost/asio/strand.hpp>
 #include <boost/asio/experimental/channel.hpp>

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

         // 核心接口：获取连接
         boost::asio::awaitable<PooledConnection> GetConnection();

     private:
         friend struct ConnectionReleaser;
         void ReturnConnection(const std::shared_ptr<PQConnection>& conn_sh_ptr);


         using WaiterChannel = boost::asio::experimental::channel<void(boost::system::error_code, std::shared_ptr<PQConnection>)>;


         const std::shared_ptr<boost::asio::io_context> ioc_;
         const std::string conn_str_;
         const int pool_size_;

         // Strand 串行调度器
         boost::asio::strand<boost::asio::io_context::executor_type> strand_;

         // 空闲连接池
         std::deque<std::shared_ptr<PQConnection>> pool_;

         // 维护等待者队列 (利用 Channel 内部公平队列)
         WaiterChannel waiters_channel_;
     };
 }