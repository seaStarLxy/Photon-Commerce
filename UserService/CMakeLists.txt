cmake_minimum_required(VERSION 3.20)
project(UserService LANGUAGES CXX)

# 注意协程需要 C20 标准以上
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(gRPC REQUIRED)
message(STATUS "Found gRPC: ${gRPC_FOUND}")
#find_package(Boost 1.74.0 REQUIRED CONFIG COMPONENTS context coroutine thread)

# 使用 FetchContent_Declare 之前，必须包含该模块
include(FetchContent)
FetchContent_Declare(
        uuid_v7
        GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
        GIT_TAG        v1.2.3
)
FetchContent_MakeAvailable(uuid_v7)

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
set(CALL_DATA_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/call_data/src/register_call_data.cc"
)

set(SERVICE_FILES

)

set(REPOSITORY_FILES

)

set(SERVER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/server/user_service_server.cc"
)

# 添加主程序可执行文件
add_executable(UserServiceServer
        main.cc
        ${CALL_DATA_FILES}
        ${SERVICE_FILES}
        ${REPOSITORY_FILES}
        ${SERVER_FILES}
)

# 添加头文件包含目录
target_include_directories(UserServiceServer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接相关库
target_link_libraries(UserServiceServer PUBLIC
        my_proto_lib        # 链接自己生成的 proto 代码库
        gRPC::grpc++
)