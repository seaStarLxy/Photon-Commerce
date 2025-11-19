// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include "../include/user_repository.h"

using namespace user_service::infrastructure;
using namespace user_service::domain;

UserRepository::UserRepository(const std::shared_ptr<UserDao>& user_dao): user_dao_(user_dao) {

}

UserRepository::~UserRepository() = default;

boost::asio::awaitable<User> UserRepository::GetUserByPhoneNumber(const std::string& phoneNumber) {

}