// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once


#include "call_data/interface/i_call_data.h"
#include <UserService/v1/user_service.grpc.pb.h>
#include <boost/asio/co_spawn.hpp>
#include <grpcpp/completion_queue.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/async_unary_call.h>


namespace user_service {
    class RegisterCallData: public ICallData {
    public:
        RegisterCallData(v1::UserService::AsyncService* service, grpc::ServerCompletionQueue* cq, boost::asio::io_context& ioc);

        void Init();

        void Proceed() override;

    private:
        void HandleCreate();

        void HandleProcess();

        void HandleFinish();

        boost::asio::awaitable<void> RunLogic();

        void OnLogicFinished(std::exception_ptr e);

        v1::UserService::AsyncService* service_;
        grpc::ServerCompletionQueue* cq_;
        grpc::ServerContext ctx_;
        v1::RegisterRequest request_;
        v1::RegisterResponse reply_;
        grpc::ServerAsyncResponseWriter<v1::RegisterResponse> responder_;

        boost::asio::io_context& ioc_;

        enum class State { CREATE, PROCESS, FINISH };
        State status_;
    };
}