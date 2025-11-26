// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#pragma once
#include <boost/asio.hpp>
#include <optional>
#include <expected>
#include "domain/user.h"
#include "infrastructure/persistence/postgresql/include/db_error.h"

namespace user_service::domain
{
    class IUserRepository
    {
    public:
        virtual ~IUserRepository() = default;
        virtual boost::asio::awaitable<std::expected<std::optional<User>, infrastructure::DbError>> GetUserByPhoneNumber(const std::string& phoneNumber) = 0;
    };
}
