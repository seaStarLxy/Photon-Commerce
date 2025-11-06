// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <libpq-fe.h>
#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <memory>

namespace common::db {
    using PGResultPtr = std::unique_ptr<PGresult, decltype(&PQclear)>;

    class PQConnection : public std::enable_shared_from_this<PQConnection> {
    public:
        explicit PQConnection(boost::asio::io_context &ioc);

        boost::asio::awaitable<void> AsyncConnect(const std::string &conn_str);

        boost::asio::awaitable<PGResultPtr> AsyncExecParams(const std::string &query,
                                                              const std::vector<std::string> &params);

    private:
        // 维护数据库连接，
        std::unique_ptr<PGconn, decltype(&PQfinish)> conn_;
        // boost提供的描述符管理器
        boost::asio::posix::stream_descriptor socket_;
    };
}
