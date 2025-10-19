// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <string>
#include <grpcpp/completion_queue.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/status.h>
#include <grpcpp/support/async_unary_call.h>

#include "call_data/interface/i_call_data.h"
#include <UserService/v1/user_service.grpc.pb.h>


namespace UserService {
    class RegisterCallData: public ICallData {
    public:
        RegisterCallData(v1::UserService::AsyncService* service, grpc::ServerCompletionQueue* cq)
            : service_(service), cq_(cq), responder_(&ctx_), status_(State::CREATE) {
            // 一被创建就驱动状态机
            Proceed();
        }

        void Proceed() override {
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

    private:
        void HandleCreate() {
            status_ = State::PROCESS;
            service_->RequestRegister(&ctx_, &request_, &responder_, cq_, cq_, this);
        }

        void HandleProcess() {
            new RegisterCallData(service_, cq_);

            // 业务逻辑
            std::string prefix("Hello, ");
            reply_.set_user_id(prefix + request_.username());

            status_ = State::FINISH;
            responder_.Finish(reply_, grpc::Status::OK, this);
        }

        void HandleFinish() {
            delete this;
        }

        // 成员变量
        v1::UserService::AsyncService* service_;
        grpc::ServerCompletionQueue* cq_;
        grpc::ServerContext ctx_;

        // 专门用于 SayHello 的消息类型
        v1::RegisterRequest request_;
        v1::RegisterResponse reply_;
        grpc::ServerAsyncResponseWriter<v1::RegisterResponse> responder_;

        enum class State { CREATE, PROCESS, FINISH };
        State status_;
    };
} // UserService