// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "../include/pq_connection.h"
#include <spdlog/spdlog.h>

using namespace user_service::infrastructure;

PQConnection::PQConnection(boost::asio::io_context &ioc) : conn_(nullptr, &PQfinish), socket_(ioc) {
}

boost::asio::awaitable<void> PQConnection::AsyncConnect(const std::string &conn_str) {
    // 异步启动连接
    conn_.reset(PQconnectStart(conn_str.c_str()));
    if (!conn_ || PQstatus(conn_.get()) == CONNECTION_BAD) {
        throw std::runtime_error(std::string("Connection failed: ") + PQerrorMessage(conn_.get()));
    }
    // 获取数据库底层描述符，并交给 boost 协程监听
    socket_.assign(PQsocket(conn_.get()));

    // 轮询连接状态直到成功，while负责推进前面的
    while (true) {
        // 获取当前数据库状态，同时也负责驱动读写操作
        const auto poll_status = PQconnectPoll(conn_.get());
        if (poll_status == PGRES_POLLING_WRITING) {
            // param1: 等待可以写操作， param2: 以协程的方式
            co_await socket_.async_wait(boost::asio::posix::stream_descriptor::wait_write, boost::asio::use_awaitable);
        } else if (poll_status == PGRES_POLLING_READING) {
            co_await socket_.async_wait(boost::asio::posix::stream_descriptor::wait_read, boost::asio::use_awaitable);
        } else if (poll_status == PGRES_POLLING_OK) {
            SPDLOG_DEBUG("Connected to Postgresql successfully!");
            co_return; // 连接成功
        } else {
            throw std::runtime_error(std::string("Async connection failed: ") + PQerrorMessage(conn_.get()));
        }
    }
}

boost::asio::awaitable<std::expected<PGResultPtr, DbError>> PQConnection::AsyncExecParams(const std::string &query,
                                                                    const std::vector<std::string> &params) {
    // 1. 发送
    SendQuery(query, params);
    // 2. 等待
    co_await AwaitResponse();
    // 3. 取值
    auto raw_result = FetchRawResult();
    // 4. 映射 (判断成功还是失败)
    co_return MapResultToExpected(std::move(raw_result));
}

/* AsyncExecParams 子函数 */

void PQConnection::SendQuery(const std::string &query, const std::vector<std::string> &params) {
    // 维护参数列表
    std::vector<const char *> param_values;
    param_values.reserve(params.size());
    for (const auto &p: params) {
        param_values.push_back(p.c_str());
    }

    // 非阻塞查询 （严重错误）
    if (PQsendQueryParams(conn_.get(), query.c_str(), params.size(), nullptr,
                          param_values.data(), nullptr, nullptr, 0) == 0) {
        // 严重错误（网络/OOM等）,当下阶段选择抛出异常
        throw std::runtime_error(std::string("Failed to send query: ") + PQerrorMessage(conn_.get()));
    }
}

boost::asio::awaitable<void> PQConnection::AwaitResponse() {
    while (true) {
        // 以协程方式监听数据库给出的反馈
        co_await socket_.async_wait(boost::asio::posix::stream_descriptor::wait_read, boost::asio::use_awaitable);
        // 真正读取数据，成功则为 1，失败则为 0
        if (PQconsumeInput(conn_.get()) == 0) {
            throw std::runtime_error(std::string("Failed to consume input: ") + PQerrorMessage(conn_.get()));
        }
        // 当前PQ连接不忙代表数据接受完整（可以安全的用 PQgetResult 接受结果了）
        if (PQisBusy(conn_.get()) == 0) {
            break;
        }
    }
}

PGResultPtr PQConnection::FetchRawResult() {
    // 处理返回结果
    PGResultPtr final_result(nullptr, &PQclear);
    // 这里用循环是因为，保证把数据全取出来，只有第一次是有效数据，但是第二次必须把 nullptr 取出来，否则本次连接未结束，下次无法使用
    while (true) {
        // 从pq底层结构拿一个结果
        PGResultPtr temp_res(PQgetResult(conn_.get()), &PQclear);

        // 结束信号：如果是 nullptr，说明连接空闲了，可以退出了
        if (!temp_res) {
            break;
        }
        // 单语句查询，只有第一个结果就是我们要的
        if (!final_result) {
            final_result = std::move(temp_res);
        }
    }
    return final_result;
}

std::expected<PGResultPtr, DbError> PQConnection::MapResultToExpected(PGResultPtr result) {
    // 防御性检查
    if (!result) {
        return PGResultPtr(nullptr, &PQclear);
    }
    const ExecStatusType status = PQresultStatus(result.get());

    // 检查 sql 运行是否发生错误（约束错误等）
    if (status == PGRES_BAD_RESPONSE || status == PGRES_FATAL_ERROR) {
        // 获取错误信息
        std::string err_msg = PQresultErrorMessage(result.get());
        // 获取 SQLState （错误代码）
        const char* sql_state_ptr = PQresultErrorField(result.get(), PG_DIAG_SQLSTATE);
        std::string sql_state = sql_state_ptr ? sql_state_ptr : "";
        SPDLOG_WARN("SQL Execution Failed. State: {}, Msg: {}", sql_state, err_msg);
        // 返回错误对象
        return std::unexpected(DbError{DbErrorType::SqlExecutionError, err_msg, sql_state});
    }
    // PGRES_TUPLES_OK 代表执行成功并返回了成功的结果，比如: select
    // PGRES_COMMAND_OK 代表执行语句执行成功（没有返回结果的那种语句），比如: update、insert
    if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK) {
        return result; // 成功，返回结果
    }

    // 其他状态返回空结果集
    return PGResultPtr(nullptr, &PQclear);
}
