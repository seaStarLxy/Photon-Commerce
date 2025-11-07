// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <string>
#include <stdexcept>
#include <libpq-fe.h>
#include <utility> // for std::move

namespace user_service::model {
    class User {
    public: // 公共接口
        // ===============================================
        // "M" (Mapping) 部分：工厂函数
        // ===============================================
        static User from_pg_result(PGresult *res, int row_num) {
            if (!res || row_num >= PQntuples(res)) {
                throw std::out_of_range("Row number is out of range for User mapping.");
            }
            if (PQnfields(res) < 2) {
                throw std::runtime_error("Not enough columns to map to User object.");
            }

            // 1. 先把数据解析出来
            int parsed_id = std::stoi(PQgetvalue(res, row_num, 0));
            std::string parsed_username = PQgetvalue(res, row_num, 1);

            // 2. 通过私有构造函数创建对象
            // 这样可以确保所有 User 对象都是通过有效途径创建的
            return User(parsed_id, std::move(parsed_username));
        }

        // ===============================================
        // "O" (Object) 部分：公共访问器 (Getters)
        // ===============================================
        // 我们只提供 "get" 方法，不提供 "set" 方法
        // 这使得从数据库加载后的 User 对象变成“只读”的，非常安全

        int get_id() const {
            return id;
        }

        const std::string &get_username() const {
            return username;
        }

        // 如果你需要允许修改用户名，你可以添加一个 Setter
        // void set_username(std::string new_name) {
        //     if (new_name.empty()) {
        //         throw std::runtime_error("Username cannot be empty.");
        //     }
        //     this->username = std::move(new_name);
        // }


    private: // 私有数据
        // ===============================================
        // "O" (Object) 部分：私有数据成员
        // ===============================================
        int id;
        std::string username;

        // ===============================================
        // 私有构造函数
        // ===============================================
        // 只有本类（包括静态成员函数）才能调用它
        User(int id, std::string username)
            : id(id), username(std::move(username)) {
            // 可以在这里添加额外的验证逻辑
            if (id <= 0) {
                throw std::runtime_error("Invalid ID for User.");
            }
        }
    };
}
