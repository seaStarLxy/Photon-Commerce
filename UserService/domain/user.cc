// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "domain/user.h"

using namespace user_service::domain;
using json = nlohmann::json;

void User::ChangePassword(const std::string& new_hash, const std::string& new_salt) {
    if (new_hash.empty() || new_salt.empty()) {
        throw std::invalid_argument("Hash and salt cannot be empty");
    }
    password_hash_ = new_hash;
    salt_ = new_salt;
}

void User::UpdateProfile(const std::optional<std::string>& username,
                   const std::optional<std::string>& email,
                   const std::optional<std::string>& avatar_url) {
    if (username.has_value()) username_ = username;
    if (email.has_value()) email_ = email;
    if (avatar_url.has_value()) avatar_url_ = avatar_url;
}

void User::MarkAsDeleted() {
    status_ = UserStatus::DELETED;
    deleted_at_ = std::chrono::system_clock::now();
}

nlohmann::json User::ToJson() const {
    return json{
            {"id", id_},
            {"phone", phone_number_},
            {"pwd_hash", password_hash_}, // 缓存中通常包含哈希以便后续逻辑验证，或视安全策略决定
            {"salt", salt_},
            {"username", username_.value_or("")},
            {"email", email_.value_or("")},
            {"avatar", avatar_url_.value_or("")},
            {"status", static_cast<int>(status_)},
            // 时间转为时间戳存储，通用性最强
            {"created_at", std::chrono::duration_cast<std::chrono::seconds>(created_at_.time_since_epoch()).count()}
    };
}

std::optional<User> User::FromJson(const nlohmann::json& j) {
    try {
        User u;
        // 必填字段，缺失则抛异常捕获
        u.id_ = j.at("id").get<std::string>();
        u.phone_number_ = j.at("phone").get<std::string>();
        u.password_hash_ = j.at("pwd_hash").get<std::string>();
        u.salt_ = j.at("salt").get<std::string>();
        u.status_ = static_cast<UserStatus>(j.at("status").get<int>());

        // 选填字段
        if (j.contains("username") && !j["username"].empty()) u.username_ = j["username"];
        if (j.contains("email") && !j["email"].empty()) u.email_ = j["email"];
        if (j.contains("avatar") && !j["avatar"].empty()) u.avatar_url_ = j["avatar"];

        // 时间恢复
        const long long ts = j.at("created_at").get<long long>();
        u.created_at_ = std::chrono::system_clock::time_point(std::chrono::seconds(ts));

        return u;
    } catch (const json::exception& e) {
        SPDLOG_ERROR("User JSON deserialization failed: {}", e.what());
        return std::nullopt;
    }
}