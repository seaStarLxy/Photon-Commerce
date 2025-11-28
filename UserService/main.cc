// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include <spdlog/spdlog.h>
#include <iostream>
#include "server/application.h"

using namespace user_service::server;
namespace di = boost::di;

void LogInit() {
    // spdlog::set_level(spdlog::level::trace);
    spdlog::set_level(spdlog::level::critical);
    // spdlog::set_level(spdlog::level::off);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] [%s:%# (%!)] %v");
}

int main() {
    LogInit();
    try {
        // 加载配置并启动应用
        const Application app("config/config.yaml");
        app.Run();
    }
    catch (const std::exception& e) {
        SPDLOG_CRITICAL("CRITICAL ERROR: Application crashed with exception: {}", e.what());
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        // 捕获非标准异常
        SPDLOG_CRITICAL("CRITICAL ERROR: Application crashed with unknown exception.");
        std::cerr << "CRITICAL ERROR: Unknown exception occurred." << std::endl;
        return -1;
    }
    return 0;
}
