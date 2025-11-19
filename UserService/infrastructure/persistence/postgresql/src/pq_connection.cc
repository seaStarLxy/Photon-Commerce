// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

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

boost::asio::awaitable<PGResultPtr> PQConnection::AsyncExecParams(const std::string &query,
                                                                    const std::vector<std::string> &params) {
    // 维护参数列表
    std::vector<const char *> param_values;
    param_values.reserve(params.size());
    for (const auto &p: params) {
        param_values.push_back(p.c_str());
    }
    // 非阻塞查询
    if (PQsendQueryParams(conn_.get(), query.c_str(), params.size(), nullptr,
                          param_values.data(), nullptr, nullptr, 0) == 0) {
        throw std::runtime_error(std::string("Failed to send query: ") + PQerrorMessage(conn_.get()));
    }
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

    // 处理返回结果
    PGResultPtr result(nullptr, &PQclear);
    while (true) {
        // 把结果从 pq的数据结构 中拷贝出来
        result.reset(PQgetResult(conn_.get()));
        // 有些操作可能有多个返回结果，所以需要使用 while循环 配合 result 检查
        if (!result) {
            // 如果结果为空指针，表示所有结果都已处理完毕
            break;
        }
        ExecStatusType status = PQresultStatus(result.get());
        // 检查 sql运行 是否错误
        if (status == PGRES_BAD_RESPONSE || status == PGRES_FATAL_ERROR) {
            // 拿到错误信息并抛出异常
            throw std::runtime_error(std::string("SQL Error: ") + PQresultErrorMessage(result.get()));
        }
        // PGRES_TUPLES_OK 代表执行成功并返回了成功的结果，比如: select
        // PGRES_COMMAND_OK 代表执行语句执行成功（没有返回结果的那种语句），比如: update、insert
        if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK) {
            co_return result; // 成功，返回结果
        }
    }
    // 循环结束还没有返回，很可能是出错了或没有返回结果集
    co_return PGResultPtr(nullptr, &PQclear);
}
