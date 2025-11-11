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
        explicit RegisterCallData(ICallDataManager* manager);
        ~RegisterCallData() override;

    private:
        boost::asio::awaitable<void> RunLogic() override;
    };
}
