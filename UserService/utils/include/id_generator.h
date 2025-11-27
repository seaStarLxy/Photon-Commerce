// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "utils/interface/i_id_generator.h"

namespace user_service::util {
    class IdGenerator: public IIDGenerator {
    public:
        IdGenerator() = default;
        ~IdGenerator() override = default;

        // 生成 UUIDv7
        std::string GenerateUUID() override;
    };
}