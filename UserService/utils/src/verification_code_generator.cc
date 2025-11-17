// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/verification_code_generator.h"

using namespace user_service::util;

CodeGenerator::~CodeGenerator() = default;

std::string CodeGenerator::Generate(int length) {
    std::uniform_int_distribution<int> distribution(0, 9);

    std::string code;
    code.reserve(length);

    for (int i = 0; i < length; ++i) {
        // 获取当前线程的引擎 (无锁)
        std::mt19937& engine = GetThreadLocalEngine();
        // 生成一个数字 (0-9)
        int digit = distribution(engine);
        code.push_back(digit + '0');
    }

    return code;
}

std::mt19937& CodeGenerator::GetThreadLocalEngine() {
    static thread_local std::mt19937 engine(std::random_device{}());
    return engine;
}