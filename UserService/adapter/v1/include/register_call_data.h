// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once


#include "../interface/i_call_data.h"
#include <UserService/v1/user_service.grpc.pb.h>
#include "service/interface/i_basic_user_service.h"
#include <boost/asio/co_spawn.hpp>
#include <grpcpp/completion_queue.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/async_unary_call.h>


namespace user_service::adapter {
    class RegisterCallData: public ICallData {
    public:
        RegisterCallData(v1::UserService::AsyncService* service, grpc::ServerCompletionQueue* cq,
            boost::asio::io_context& ioc, const std::shared_ptr<service::IBasicUserService>& basic_user_service);

        void Init();

        void Proceed() override;

    private:
        void HandleCreate();

        void HandleProcess();

        void HandleFinish();

        boost::asio::awaitable<void> RunLogic();

        void OnLogicFinished(std::exception_ptr e);

        // grpc 所需变量
        v1::UserService::AsyncService* service_;
        grpc::ServerCompletionQueue* cq_;
        grpc::ServerContext ctx_;
        grpc::ServerAsyncResponseWriter<v1::RegisterResponse> responder_;

        v1::RegisterRequest request_;
        v1::RegisterResponse reply_;

        enum class State { CREATE, PROCESS, FINISH };
        State status_;


        // 协程所需变量
        boost::asio::io_context& ioc_;

        std::shared_ptr<service::IBasicUserService> basic_user_service_;


    };
}