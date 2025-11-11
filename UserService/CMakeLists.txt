cmake_minimum_required(VERSION 3.20)
project(UserService LANGUAGES CXX)

# 注意协程需要 C20 标准以上
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(gRPC REQUIRED)
message(STATUS "Found gRPC: ${gRPC_FOUND}")
find_package(PostgreSQL REQUIRED)
message(STATUS "Found PostgreSQL: ${PostgreSQL_FOUND}")
find_package(Boost 1.85.0 REQUIRED COMPONENTS redis)

# 使用 FetchContent_Declare 之前，必须包含该模块
include(FetchContent)

FetchContent_Declare(
        uuid_v7
        GIT_REPOSITORY https://github.com/seaStarLxy/stduuid.git
        GIT_TAG        v1.2.3
)
FetchContent_MakeAvailable(uuid_v7)

FetchContent_Declare(
        bcrypt
        GIT_REPOSITORY https://github.com/starlxy123/Bcrypt.cpp
        GIT_TAG master
)
FetchContent_MakeAvailable(bcrypt)

FetchContent_Declare(
        boost_di
        GIT_REPOSITORY https://github.com/seaStarLxy/di.git
        GIT_TAG v1.3.2
)
FetchContent_MakeAvailable(boost_di)

FetchContent_Declare(
        jwt-cpp
        GIT_REPOSITORY https://github.com/seaStarLxy/jwt-cpp.git
        GIT_TAG v0.6.0
)
FetchContent_MakeAvailable(jwt-cpp)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/seaStarLxy/spdlog.git
        GIT_TAG        v1.16.0
)
FetchContent_MakeAvailable(spdlog)
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)


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
)

set(ADAPTER_FILES
#        v1
#        "adapter/v1/src/register_call_data.cc"
#         v2
#        "adapter/v2/object_pool/call_data_manager.cc"
        "adapter/v2/src/register_call_data.cc"
)

set(SERVICE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/service/src/auth_service.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/service/src/basic_user_service.cc"
)

set(DOMAIN_FILES

)

set(INFRASTRUCTURE_FILES
        # dao
        # redis
        # db
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/db/src/pq_connection.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/db/src/async_connection_pool.cc"
        # asio_thread_pool
        "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/asio_thread_pool/asio_thread_pool.cc"
)

set(UTILS_FILES

)

# 添加主程序可执行文件
add_executable(UserServiceServer
        main.cc
        ${SERVER_FILES}
        ${ADAPTER_FILES}
        ${SERVICE_FILES}
        ${DOMAIN_FILES}
        ${INFRASTRUCTURE_FILES}
        ${DB_FILES}
        ${UTILS_FILES}
)

# 添加头文件包含目录
target_include_directories(UserServiceServer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接相关库
target_link_libraries(UserServiceServer PUBLIC
        my_proto_lib        # 链接自己生成的 proto 代码库
        gRPC::grpc++
        bcrypt
        jwt-cpp
        PostgreSQL::PostgreSQL
        spdlog::spdlog
        Boost.DI
        Boost::redis
)