// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "service/interface/i_auth_service.h"
#include "utils/interface/i_verification_code_generator.h"
#include "domain/interface/i_verification_code_repository.h"
#include "domain/interface/i_user_repository.h"
#include "utils/interface/i_jwt_util.h"
#include "utils/interface/i_security_util.h"

namespace user_service::service {
    class AuthService final: public IAuthService {
    public:
        explicit AuthService(const std::shared_ptr<util::IVerificationCodeGenerator>& code_generator,
            const std::shared_ptr<domain::IVerificationCodeRepository>& code_repository,
            const std::shared_ptr<domain::IUserRepository>& user_repository,
            const std::shared_ptr<util::IJwtUtil>& jwt_util,
            const std::shared_ptr<util::ISecurityUtil>& security_util);
        ~AuthService() override;
        boost::asio::awaitable<SendCodeResponse> SendCode(const SendCodeRequest&) override;
        boost::asio::awaitable<LoginResult> LoginByCode(const LoginByCodeRequest&) override;
        boost::asio::awaitable<LoginResult> LoginByPassword(const LoginByPasswordRequest&) override;
    private:
        std::shared_ptr<util::IVerificationCodeGenerator> verification_code_generator_;
        std::shared_ptr<domain::IVerificationCodeRepository> verification_code_repository_;
        std::shared_ptr<domain::IUserRepository> user_repository_;
        std::shared_ptr<util::IJwtUtil> jwt_util_;
        std::shared_ptr<util::ISecurityUtil> security_util_;
    };
}
