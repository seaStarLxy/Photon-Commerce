// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "../include/user_repository.h"

using namespace user_service::infrastructure;
using namespace user_service::domain;

UserRepository::UserRepository(const std::shared_ptr<UserDao>& user_dao): user_dao_(user_dao) {

}

UserRepository::~UserRepository() = default;

boost::asio::awaitable<std::expected<std::optional<User>, DbError>> UserRepository::GetUserByPhoneNumber(const std::string& phoneNumber) {
    co_return co_await user_dao_->GetUserByPhoneNumber(phoneNumber);
}