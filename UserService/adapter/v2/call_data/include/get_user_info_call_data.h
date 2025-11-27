// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "adapter/v2/call_data/interface/call_data.hpp"
#include <UserService/v1/user_service.grpc.pb.h>

namespace user_service::adapter::v2 {
    class GetUserInfoCallDataManager;

    class GetUserInfoCallData final: public CallData<proto::v1::GetUserInfoRequest, proto::v1::GetUserInfoResponse, GetUserInfoCallDataManager, GetUserInfoCallData> {
        friend GetUserInfoCallDataManager;
    public:
        explicit GetUserInfoCallData(GetUserInfoCallDataManager* manager);
        ~GetUserInfoCallData() override;
        boost::asio::awaitable<void> RunSpecificLogic();
    };
}