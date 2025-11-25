// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <libpq-fe.h>
#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <memory>
#include <expected>

namespace user_service::infrastructure {
    enum class DbErrorType {
        NetworkError,
        SqlExecutionError,  // 执行时错误
    };
    struct DbError {
        DbErrorType type;
        std::string pg_error_message;
        std::string sql_state;  // PGSQL 的错误代码，比如 "23505" 代表唯一键冲突
    };

    using PGResultPtr = std::unique_ptr<PGresult, decltype(&PQclear)>;

    class PQConnection : public std::enable_shared_from_this<PQConnection> {
    public:
        explicit PQConnection(boost::asio::io_context &ioc);

        boost::asio::awaitable<void> AsyncConnect(const std::string &conn_str);

        boost::asio::awaitable<std::expected<PGResultPtr, DbError>> AsyncExecParams(const std::string &query,
                                                              const std::vector<std::string> &params);

    private:
        // 1. 发送查询指令
        void SendQuery(const std::string &query, const std::vector<std::string> &params);

        // 2. 协程等待数据库响应
        boost::asio::awaitable<void> AwaitResponse();

        // 3. 循环取出结果，只保留第一个非空结果
        PGResultPtr FetchRawResult();

        // 4. 将底层结果转换为业务预期的 expected 对象
        std::expected<PGResultPtr, DbError> MapResultToExpected(PGResultPtr result);

        // 维护数据库连接，
        std::unique_ptr<PGconn, decltype(&PQfinish)> conn_;
        // boost提供的描述符管理器
        boost::asio::posix::stream_descriptor socket_;
    };
}
