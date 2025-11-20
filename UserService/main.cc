// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include <spdlog/spdlog.h>
#include "server/application.h"

using namespace user_service::server;
namespace di = boost::di;

void LogInit() {
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] [%s:%# (%!)] %v");
}

int main() {
    LogInit();
    try {
        const Application app("config/config.yaml");
        app.Run();
    } catch (...) {
        return -1;
    }
    return 0;
}
