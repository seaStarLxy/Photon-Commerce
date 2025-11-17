// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <string>

namespace user_service::util {
    class IVerificationCodeGenerator {
    public:
        virtual ~IVerificationCodeGenerator() = default;
        virtual std::string Generate(int length) = 0;
    };
}