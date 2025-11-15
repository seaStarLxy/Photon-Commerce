// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/code_generator.h"

using namespace user_service::util;

CodeGenerator::~CodeGenerator() = default;

std::string CodeGenerator::Generate(int length) {
    std::uniform_int_distribution<int> distribution(0, 9);

    std::string code;
    code.reserve(length); // 预分配内存

    for (int i = 0; i < length; ++i) {
        // 1. 调用私有静态辅助函数, 获取“本线程”的引擎 (无锁)
        std::mt19937& engine = GetThreadLocalEngine();

        // 2. 生成一个数字 (0-9)
        int digit = distribution(engine);

        // 3. 用最高效的方式追加字符
        code.push_back(digit + '0');
    }

    return code;
}

std::mt19937& CodeGenerator::GetThreadLocalEngine() {
    static thread_local std::mt19937 engine(std::random_device{}());
    return engine;
}