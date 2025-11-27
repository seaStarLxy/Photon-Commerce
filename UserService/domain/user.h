// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved

#pragma once
#include <string>
#include <optional>
#include <chrono>
#include <format>

namespace user_service::infrastructure {
    class UserDao;
}

namespace user_service::domain {
    // 对应数据库 status 字段 (SMALLINT) 指定16位存储
    enum class UserStatus : int16_t {
        NORMAL = 0,
        FROZEN = 1,
        INACTIVE = 2,
        DELETED = 99
    };

    /* 注意：这个类谨慎添加析构函数，避免破坏五之法则，导致移动构造 */
    class User {
        // 让 UserDao 可以直接映射为 User
        friend class infrastructure::UserDao;

    public:
        using TimePoint = std::chrono::system_clock::time_point;

        /* 业务行为 */
        void ChangePassword(const std::string &new_hash, const std::string &new_salt); // 修改密码

        void UpdateProfile(std::optional<std::string> username,
                           std::optional<std::string> email,
                           std::optional<std::string> avatar_url); // 修改个人资料

        void MarkAsDeleted(); // 软删除

        static User Create(std::string id, std::string phone, std::string pwd_hash, std::string salt) {
            User u;
            u.id_ = std::move(id);
            u.phone_number_ = std::move(phone);
            u.password_hash_ = std::move(pwd_hash);
            u.salt_ = std::move(salt);
            u.status_ = UserStatus::NORMAL;
            // created_at 不需要设置，插入数据库时会自动生成
            return u;
        }

        // 只读
        [[nodiscard]] const std::string &GetId() const { return id_; }
        [[nodiscard]] const std::string &GetPhoneNumber() const { return phone_number_; }
        [[nodiscard]] const std::string &GetPasswordHash() const { return password_hash_; }
        [[nodiscard]] const std::string &GetSalt() const { return salt_; }

        // Optional 字段返回 const 指针或者 optional 引用，方便调用者判断
        [[nodiscard]] std::optional<std::string> GetUsername() const { return username_; }
        [[nodiscard]] std::optional<std::string> GetEmail() const { return email_; }
        [[nodiscard]] std::optional<std::string> GetAvatarUrl() const { return avatar_url_; }

        [[nodiscard]] UserStatus GetStatus() const { return status_; }
        [[nodiscard]] int GetStatusValue() const { return static_cast<int>(status_); } // 给 DAO 存库用

        [[nodiscard]] TimePoint GetCreatedAt() const { return created_at_; }
        [[nodiscard]] std::optional<TimePoint> GetDeletedAt() const { return deleted_at_; }

        // 判断是否已删除
        [[nodiscard]] bool IsDeleted() const {
            return deleted_at_.has_value() || status_ == UserStatus::DELETED;
        }

        [[nodiscard]] std::string ToString() const {
            // 辅助 Lambda：把 time_point 转成人类可读的字符串 (UTC)
            auto fmt_time = [](const TimePoint& tp) -> std::string {
                const auto t = std::chrono::system_clock::to_time_t(tp);
                if (t == 0) return "null";

                const std::tm tm = *std::gmtime(&t); // 转为 UTC 结构体
                char buffer[32];
                // 格式化为: 2025-11-26 10:00:00
                std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
                return std::string(buffer);
            };

            return std::format(
                "User {{ "
                "id: {}, "
                "phone: {}, "
                "username: {}, "
                "email: {}, "
                "status: {}, "
                "created_at: {}, "
                "deleted_at: {} "
                "}}",
                id_,
                phone_number_,
                username_.value_or("null"),
                email_.value_or("null"),
                static_cast<int>(status_),
                fmt_time(created_at_),
                deleted_at_.has_value() ? fmt_time(*deleted_at_) : "null"
            );
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
