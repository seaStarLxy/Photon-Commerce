// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved

#pragma once
#include <string>
#include <optional>
#include <chrono>
// #include "infrastructure/persistence/dao/user_dao.h"

namespace user_service::infrastructure {
    class UserDao;
}

namespace user_service::domain
{
    // 对应数据库 status 字段 (SMALLINT) 指定16位存储
    enum class UserStatus : int16_t
    {
        NORMAL = 0,
        FROZEN = 1,
        INACTIVE = 2,
        DELETED = 99
    };
    class User
    {
        friend class infrastructure::UserDao;   // friend & 前向声明
    public:
        using TimePoint = std::chrono::system_clock::time_point;

        /* 业务行为 */
        void ChangePassword(const std::string& new_hash, const std::string& new_salt); // 修改密码

        void UpdateProfile(std::optional<std::string> username,
                           std::optional<std::string> email,
                           std::optional<std::string> avatar_url); // 修改个人资料

        void MarkAsDeleted(); // 软删除

        // 只读
        const std::string& GetId() const { return id_; }
        const std::string& GetPhoneNumber() const { return phone_number_; }
        const std::string& GetPasswordHash() const { return password_hash_; }
        const std::string& GetSalt() const { return salt_; }

        // Optional 字段返回 const 指针或者 optional 引用，方便调用者判断
        std::optional<std::string> GetUsername() const { return username_; }
        std::optional<std::string> GetEmail() const { return email_; }
        std::optional<std::string> GetAvatarUrl() const { return avatar_url_; }

        UserStatus GetStatus() const { return status_; }
        int GetStatusValue() const { return static_cast<int>(status_); } // 给 DAO 存库用

        TimePoint GetCreatedAt() const { return created_at_; }
        std::optional<TimePoint> GetDeletedAt() const { return deleted_at_; }

        // 判断是否已删除
        bool IsDeleted() const
        {
            return deleted_at_.has_value() || status_ == UserStatus::DELETED;
        }

    private:
        User() = default;

        std::string id_; // UUID
        std::string phone_number_;

        // 认证安全字段
        std::string password_hash_;
        std::string salt_;

        // 可空字段
        std::optional<std::string> username_;
        std::optional<std::string> email_;
        std::optional<std::string> avatar_url_;

        // 状态与时间
        UserStatus status_;
        TimePoint created_at_;
        std::optional<TimePoint> deleted_at_; // 软删除时间
    };
}
