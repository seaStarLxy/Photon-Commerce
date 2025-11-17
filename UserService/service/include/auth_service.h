// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "domain/interface/i_verification_code_repository.h"
#include "service/interface/i_auth_service.h"
#include "utils/interface/i_verification_code_generator.h"

namespace user_service::service {
    class AuthService final: public IAuthService {
    public:
        explicit AuthService(const std::shared_ptr<util::IVerificationCodeGenerator>& code_generator,
            const std::shared_ptr<domain::IVerificationCodeRepository>& code_repository);
        ~AuthService() override;
        boost::asio::awaitable<SendCodeResponse> SendCode(const SendCodeRequest&) override;
        boost::asio::awaitable<LoginResult> LoginByCode(const LoginByCodeRequest&) override;
        boost::asio::awaitable<LoginResult> LoginByPassword(const LoginByCodeRequest&) override;
    private:
        std::shared_ptr<util::IVerificationCodeGenerator> verification_code_generator_;
        std::shared_ptr<domain::IVerificationCodeRepository> verification_code_repository_;
    };
}
