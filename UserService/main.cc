// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

// #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "server/user_service_server.h"
#include <spdlog/spdlog.h>

using namespace user_service;

int main()
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] [%s:%# (%!)] %v");
    SPDLOG_DEBUG("Start a new UserServiceServer");
    UserServiceServer server;
    server.Run();
    return 0;
}
