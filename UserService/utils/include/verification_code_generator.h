// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "utils/interface/i_verification_code_generator.h"
#include <random>

namespace user_service::util {
    class CodeGenerator final: public IVerificationCodeGenerator{
    public:
        ~CodeGenerator() override;
        std::string Generate(int length) override;
    private:
        static std::mt19937& GetThreadLocalEngine();
    };
}
