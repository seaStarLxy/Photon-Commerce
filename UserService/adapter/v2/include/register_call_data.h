// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "../interface/i_call_data.hpp"
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>
#include <grpcpp/grpcpp.h>
#include <UserService/v1/user_service.grpc.pb.h>
#include "service/interface/i_basic_user_service.h"

namespace user_service::adapter::v2 {
    class RegisterCallData final: public ICallData<proto::v1::RegisterRequest, proto::v1::RegisterResponse> {
    public:
        explicit RegisterCallData(ICallDataManager* manager): ICallData(manager) {

        }
        ~RegisterCallData() override = default;

    private:
        boost::asio::awaitable<void> RunLogic() override {
            SPDLOG_DEBUG("run RunLogic");
            service::IBasicUserService* basic_user_service = static_cast<service::IBasicUserService*>(manager_->GetBusinessService());
            // SPDLOG_DEBUG("basic_user_service address = {}", basic_user_service);
            RegisterRequest register_request(request_.username(), request_.password(),
                request_.phone_number(), request_.code());
            SPDLOG_DEBUG("ready to enter coroutine");
            RegisterResponse register_response = co_await basic_user_service->Register(register_request);
            SPDLOG_DEBUG("leave from coroutine");
            reply_.set_user_id(register_response.user_id);
            reply_.set_token(register_response.token);
            co_return;
        }
    };
}
