// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <string>

struct RegisterRequest {
    std::string username;
    std::string password;
    std::string phone_number;
    std::string code;
};

struct RegisterResponse {
    std::string user_id;
    std::string token;
};

struct GetUserInfoRequest {
    std::string user_id;
};
struct GetUserInfoResponse {
    std::string user_id;
    std::string username;
    std::string email;
    std::string avatar_url;
};

struct UpdateUserInfoRequest {
    std::string username;
    std::string email;
    std::string avatar_url;
};

struct UpdateUserInfoResponse {
    bool status;
};