#pragma once
#include <string>
#include <chrono>

namespace Domain
{

    struct User
    {
        std::string user_id;
        std::string username;
        std::string email;
        std::string password_hash; // 数据库存储的是哈希值
        std::string avatar_url;
        std::chrono::system_clock::time_point created_at;
    };

}