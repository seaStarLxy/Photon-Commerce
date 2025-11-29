cmake_minimum_required(VERSION 3.20)
project(UserService LANGUAGES CXX)


# 编译器参数：关闭优化(-O0)，开启调试信息(-g)，开启覆盖率(--coverage)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -O0 -g")

# 链接器参数
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")


# 注意协程需要 C23 标准以上
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(gRPC REQUIRED)
message(STATUS "Found gRPC: ${gRPC_FOUND}")
find_package(PostgreSQL REQUIRED)
message(STATUS "Found PostgreSQL: ${PostgreSQL_FOUND}")
find_package(OpenSSL REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(cryptopp REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
#add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)


# 自定义 IDL 目录
set(MY_IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../IDL")
# google IDL 目录
set(GOOGLEAPIS_PROTOS_DIR "../third_party/googleapis")
# 收集 自定义IDL和其依赖 google IDL 目录中的 .proto 文件
set(PROTO_FILES
        "${MY_IDL_DIR}/UserService/v1/user_service.proto"
        "${GOOGLEAPIS_PROTOS_DIR}/google/api/annotations.proto"
        "${GOOGLEAPIS_PROTOS_DIR}/google/api/http.proto"
)
message(STATUS "Proto files: ${PROTO_FILES}")


# 生成文件存放目录
set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_BINARY_DIR})
# 注册自定义临时库目标
add_library(my_proto_lib OBJECT
        ${PROTO_FILES}
)
target_link_libraries(my_proto_lib PUBLIC protobuf::libprotobuf)
target_include_directories(my_proto_lib PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
# 1. 生成 .pb.h 和 .pb.cc
# 2. 添加到 my_proto_lib 临时库
protobuf_generate(
        TARGET my_proto_lib
        LANGUAGE cpp
        IMPORT_DIRS
        "${GOOGLEAPIS_PROTOS_DIR}"
        "${MY_IDL_DIR}"
        PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
        GENERATE_EXTENSIONS .pb.h .pb.cc
)
protobuf_generate(
        TARGET my_proto_lib
        LANGUAGE grpc
        IMPORT_DIRS
        "${GOOGLEAPIS_PROTOS_DIR}"
        "${MY_IDL_DIR}"
        PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
        PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
)


# 收集源文件
set(SERVER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/server/user_service_server.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/server/application.cc"
)

set(ADAPTER_FILES
        #        v1
        #        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v1/src/register_call_data.cc"
        #         v2
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data_manager/src/register_call_data_manager.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data/src/register_call_data.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data_manager/src/send_code_call_data_manager.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data/src/send_code_call_data.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data_manager/src/login_by_password_call_data_manager.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data/src/login_by_password_call_data.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data_manager/src/login_by_code_call_data_manager.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data/src/login_by_code_call_data.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data_manager/src/get_user_info_call_data_manager.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/v2/call_data/src/get_user_info_call_data.cc"
)

set(SERVICE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/service/src/auth_service.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/service/src/basic_user_service.cc"
)

set(DOMAIN_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/domain/user.cc"
)

set(INFRASTRUCTURE_FILES
        # domain_implement
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/domain_implement/src/verification_code_repository.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/domain_implement/src/user_repository.cc"
        # persistence
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/persistence/postgresql/src/pq_connection.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/persistence/postgresql/src/async_connection_pool.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/persistence/dao/user_dao.cc"
        # state_storage
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/state_storage/redis_dao/redis_client.cc"
        # asio_thread_pool
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/asio_thread_pool/asio_thread_pool.cc"
)

set(UTILS_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/utils/src/verification_code_generator.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/utils/src/id_generator.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/utils/src/jwt_util.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/utils/src/security_util.cc"
)

set(CONFIG_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/config/app_config.cc"
)

set(BUILTIN_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/builtin_impl/builtin_impl.cc"
)

# 添加主程序可执行文件
add_executable(UserServiceServer
        main.cc
        ${SERVER_FILES}
        ${ADAPTER_FILES}
        ${SERVICE_FILES}
        ${DOMAIN_FILES}
        ${INFRASTRUCTURE_FILES}
        ${UTILS_FILES}
        ${CONFIG_FILES}
        ${BUILTIN_FILES}
)

# 添加预编译头文件 PCH
target_precompile_headers(UserServiceServer PRIVATE
        # --- 标准库 ---
        <memory>
        <string>
        <vector>
        <future>

        # --- 巨型第三方库 (加速编译) ---
        <boost/di.hpp>
        <boost/asio.hpp>
        <spdlog/spdlog.h>
        <grpcpp/grpcpp.h>
#        <jwt-cpp/jwt.h>    # 修改了宏，不能预编译
        <stduuid/uuid.h>
        <boost/redis.hpp>
)

# 添加头文件包含目录
target_include_directories(UserServiceServer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接相关库
target_link_libraries(UserServiceServer PUBLIC
        my_proto_lib        # 链接自己生成的 proto 代码库
        gRPC::grpc++
        OpenSSL::SSL
        OpenSSL::Crypto
        PostgreSQL::PostgreSQL
        cryptopp::cryptopp
        jwt-cpp::jwt-cpp
        spdlog::spdlog
        stduuid
        yaml-cpp::yaml-cpp
        nlohmann_json::nlohmann_json
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/config.yaml
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/config)

