// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <string>

namespace user_service::util {
    class IIDGenerator {
    public:
        virtual ~IIDGenerator() = default;
        virtual std::string GenerateUUID() = 0;
    };
}