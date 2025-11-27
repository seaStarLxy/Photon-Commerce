// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include "service/interface/i_basic_user_service.h"
#include "domain/interface/i_user_repository.h"
#include "domain/interface/i_verification_code_repository.h"
#include "utils/interface/i_id_generator.h"
#include "utils/interface/i_security_util.h"

namespace user_service::service {
    class BasicUserService final: public IBasicUserService {
    public:
        BasicUserService(const std::shared_ptr<domain::IUserRepository>& user_repo,
            const std::shared_ptr<domain::IVerificationCodeRepository>& code_repo,
            const std::shared_ptr<util::IIDGenerator>& id_gen,
            const std::shared_ptr<util::ISecurityUtil>& security_util);
        ~BasicUserService() override;
        boost::asio::awaitable<RegisterResponse> Register(const RegisterRequest&) override;
        boost::asio::awaitable<GetUserInfoResponse> GetUserInfo(const GetUserInfoRequest&) override;
        boost::asio::awaitable<UpdateUserInfoResponse> UpdateUserInfo(const UpdateUserInfoRequest&) override;
    private:
        std::shared_ptr<domain::IUserRepository> user_repository_;
        std::shared_ptr<domain::IVerificationCodeRepository> code_repository_;
        std::shared_ptr<util::IIDGenerator> id_generator_;
        std::shared_ptr<util::ISecurityUtil> security_util_;
    };
}
