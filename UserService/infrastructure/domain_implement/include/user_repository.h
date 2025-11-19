// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include "domain/interface/i_user_repository.h"
#include "infrastructure/persistence/dao/user_dao.h"

namespace user_service::infrastructure {
    class UserRepository final : public domain::IUserRepository {
    public:
        explicit UserRepository(const std::shared_ptr<UserDao>& user_dao);
        ~UserRepository() override;
        boost::asio::awaitable<domain::User> GetUserByPhoneNumber(const std::string& phoneNumber) override;
    private:
        const std::shared_ptr<UserDao> user_dao_;
    };
}
