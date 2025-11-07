// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.


#include "../include/register_call_data.h"
#include <string>
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

using namespace user_service;

RegisterCallData::RegisterCallData(v1::UserService::AsyncService *service, grpc::ServerCompletionQueue *cq,
                                   boost::asio::io_context &ioc)
    : service_(service), cq_(cq), responder_(&ctx_), status_(State::CREATE), ioc_(ioc) {
}

void RegisterCallData::Init() {
    Proceed();
}

void RegisterCallData::Proceed() {
    switch (status_) {
        case State::CREATE:
            HandleCreate();
            break;
        case State::PROCESS:
            HandleProcess();
            break;
        case State::FINISH:
            HandleFinish();
            break;
    }
}

void RegisterCallData::HandleCreate() {
    status_ = State::PROCESS;
    service_->RequestRegister(&ctx_, &request_, &responder_, cq_, cq_, this);
}

void RegisterCallData::HandleProcess() {
    (new RegisterCallData(service_, cq_, ioc_))->Init();
    /*
     * 2个陷阱：
     *  1.状态改变不能由asio线程执行，必须是工作线程
     *      zh
     *  2.状态改变需要放在启动协程前
     *
     */
    status_ = State::FINISH;
    SPDLOG_DEBUG("start register coroutine");
    boost::asio::co_spawn(ioc_,
        [this] { return RunLogic(); },
        [this](std::exception_ptr e) { OnLogicFinished(e); }
    );
}

void RegisterCallData::HandleFinish() {
    delete this;
}

boost::asio::awaitable<void> RegisterCallData::RunLogic() {
    SPDLOG_DEBUG("run RunLogic");
    std::string prefix("Hello, ");
    reply_.set_user_id(prefix + request_.username());
    co_return;
}

void RegisterCallData::OnLogicFinished(std::exception_ptr e) {
    SPDLOG_DEBUG("run OnLogicFinished");
    grpc::Status status;

    if (e) {
        std::string error_message = "Internal server error";
        try {
            std::rethrow_exception(e);
        } catch (const std::exception &ex) {
            error_message = ex.what();
            SPDLOG_ERROR("Coroutine finished with error: {}", error_message);
        } catch (...) {
            error_message = "Unknown exception type";
            SPDLOG_ERROR("Coroutine finished with non-standard exception.");
        }
        status = grpc::Status(grpc::StatusCode::INTERNAL, error_message);
    } else {
        // 协程成功完成
        SPDLOG_DEBUG("Coroutine finished successfully for user: {}", request_.username());
        status = grpc::Status::OK;
    }

    // 调用 Finish
    responder_.Finish(reply_, status, this);
}
