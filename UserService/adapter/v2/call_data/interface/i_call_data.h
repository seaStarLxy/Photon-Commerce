// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <boost/asio/co_spawn.hpp>
#include <spdlog/spdlog.h>
#include <grpcpp/grpcpp.h>

#include "adapter/v2/call_data_manager/interface/call_data_manager.hpp"

namespace user_service::adapter::v2 {
    class ICallData {
    public:
        explicit ICallData() {}
        virtual ~ICallData() = default;

        // HandleRpc 中 CQ 给出返回 calldata 时调用，用于驱动 calldata
        virtual void Proceed(bool ok) = 0;
    };


}
