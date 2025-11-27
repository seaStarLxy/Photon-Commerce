// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "utils/include/id_generator.h"

using namespace user_service::util;

std::string IdGenerator::GenerateUUID() {
    // 1. 获取当前毫秒级时间戳 (48 bits)
    const auto now = std::chrono::system_clock::now();
    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // 2. 准备随机数生成器 (复用 generator 避免重建开销)
    static thread_local std::mt19937 generator = [] {
        std::random_device rd;
        std::seed_seq seq{rd(), rd(), rd(), rd()};
        return std::mt19937(seq);
    }();

    // 使用 std::uniform_int_distribution 生成随机字节
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    // 3. 填充 16 字节 buffer
    // stduuid 内部通常维护一个 16 字节的 array 或 span
    std::array<uint8_t, 16> buffer{};

    // --- 填充时间戳 (Big Endian) ---
    buffer[0] = (timestamp >> 40) & 0xFF;
    buffer[1] = (timestamp >> 32) & 0xFF;
    buffer[2] = (timestamp >> 24) & 0xFF;
    buffer[3] = (timestamp >> 16) & 0xFF;
    buffer[4] = (timestamp >> 8) & 0xFF;
    buffer[5] = timestamp & 0xFF;

    // --- 填充剩余随机数据 (10 字节) ---
    // 为了效率，分块生成随机数
    const uint32_t rand1 = dist(generator);
    const uint32_t rand2 = dist(generator);
    const uint32_t rand3 = dist(generator); // 只用部分

    buffer[6] = (rand1 >> 24) & 0xFF;
    buffer[7] = (rand1 >> 16) & 0xFF;
    buffer[8] = (rand1 >> 8) & 0xFF;
    buffer[9] = rand1 & 0xFF;

    buffer[10] = (rand2 >> 24) & 0xFF;
    buffer[11] = (rand2 >> 16) & 0xFF;
    buffer[12] = (rand2 >> 8) & 0xFF;
    buffer[13] = rand2 & 0xFF;

    buffer[14] = (rand3 >> 24) & 0xFF;
    buffer[15] = (rand3 >> 16) & 0xFF;

    // --- 设置 Version 和 Variant ---

    // Version 7: 0111 (即 0x70)
    // 位于第 7 个字节 (buffer[6]) 的高 4 位
    buffer[6] = (buffer[6] & 0x0F) | 0x70;

    // Variant 2: 10xx (即 0x80)
    // 位于第 9 个字节 (buffer[8]) 的高 2 位
    buffer[8] = (buffer[8] & 0x3F) | 0x80;

    // 4. 构造 uuids::uuid 对象并返回字符串
    // 使用 std::span 或迭代器构造 (取决于 stduuid 版本，迭代器最通用)
    const uuids::uuid id(buffer.begin(), buffer.end());

    return uuids::to_string(id);
}