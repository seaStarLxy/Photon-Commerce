# Photon-Commerce

[**🇺🇸 English**](./README.md) | [**🇨🇳 中文文档**](./README_CN.md)

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![C++ Version](https://img.shields.io/badge/C++-23-blue)
![Architecture](https://img.shields.io/badge/Architecture-Microservices-orange)
![Communication](https://img.shields.io/badge/gRPC-Asynchronous-red)
![Gateway](https://img.shields.io/badge/Envoy-Proxy-purple)
![License](https://img.shields.io/badge/License-MIT-green.svg)

**Photon-Commerce** 是一个基于 **C++23** 和 **gRPC** 构建的高性能、云原生分布式电商平台引擎。

本项目旨在探索极致的后端性能与架构设计，融合了 **协程**、**无锁编程**、**领域驱动设计** 以及 **Saga 分布式事务** 等先进技术理念。

---

## 🚀 核心特性

### ⚡️️ 极致性能与异步架构
* **全链路异步化**：基于 gRPC 异步模型与 C++20 协程，实现了高并发下的低延迟处理，拒绝 I/O 阻塞。
* **CallData 状态机引擎**：设计了基于 3 层继承与模板元编程的 `CallData` 封装，结合 **CRTP (奇异递归模板模式)**，实现了编译期多态与内存极致优化。
* **对象池化技术**：实现了通用的 `Manager` 对象池模板，复用 `CallData` 对象，大幅减少内存分配 (new/delete) 开销。
* **零开销抽象**：利用 `Boost.DI` 实现依赖注入与控制反转，在保证代码解耦的同时维持运行时性能。

### 🧩 分布式与微服务治理
* **API 网关集成**：集成 **Envoy** 作为流量入口，自动实现 RESTful API 到 gRPC 的协议转换。
* **数据一致性**：采用 **Saga 模式** 解决微服务间的数据一致性问题，保障分布式事务的最终一致性。
* **领域驱动设计**：采用 **充血模型** 设计业务逻辑，实现清晰的架构分层。
* **异步数据库交互**：基于 `libpq` 封装了协程化的异步数据库连接池，高效处理持久化需求。

---

## 📐 系统架构与核心设计

### 1. 总体架构图
![System Architecture](docs/images/zh/system_architecture_zh.png)

### 2. CallData 异步状态机设计
![CallData Design](docs/images/zh/grpc_async_workflow_zh.png)

---

## ⚙️ 技术栈概览

| 维度         | 选型方案                 | 核心考量                        |
|:-----------|:---------------------|:----------------------------|
| **开发语言**   | **C++23**            | Concepts, Coroutines, Boost |
| **通信协议**   | **gRPC & Protobuf**  | 高性能 RPC，异步流式处理              |
| **API 网关** | **Envoy Proxy**      | 流量治理，REST 转 gRPC            |
| **数据存储**   | **PostgreSQL**       | 关系型数据持久化                    |
| **缓存系统**   | **Redis**            | 热点数据缓存与状态管理                 |
| **依赖管理**   | **vcpkg & CMake**    | 现代 C++ 包管理与构建系统             |
| **容器化**    | **Docker**           | 标准化开发与运行环境                  |
| **部署架构**   | **Kubernetes (K8s)** | 服务编排与自动扩缩容                  |

### 关键依赖库
| 库名称          | 用途              | 说明                                                 |
|:-------------|:----------------|:---------------------------------------------------|
| `googleapis` | API Transcoding | 提供 HTTP/JSON 到 gRPC 的映射支持                          |
| `Boost`      | 核心组件            | 包含 `Asio` (网络/协程调度) 及 `Boost.Redis` (异步 Redis 客户端) |
| `Boost.DI`   | 依赖注入            | 现代 C++ 编译期依赖注入库 (零开销控制反转)                          |
| `libpq`      | 数据库驱动           | PostgreSQL 底层 C 接口，已封装为异步                          |
| `moodycamel` | 并发数据结构          | 高性能无锁队列 (ConcurrentQueue)                          |
| `spdlog`     | 日志系统            | 极速 C++ 日志库                                         |
| `cryptopp`   | 安全加密            | 密码哈希与加解密                                           |
| `jwt-cpp`    | 身份认证            | JSON Web Token 处理                                  |
| `uuid_v7`    | ID 生成           | 分布式唯一 ID 管理                                        |
| `yaml-cpp`   | 配置管理            | YAML 配置文件解析                                        |
---

## ▶️ 快速开始

> 本指南用于帮助在本地快速搭建开发环境。

### 1. 获取源码
**注意**：务必使用 `--recursive` 参数以包含所有子模块。

```bash
# 首次克隆
git clone --recursive git@github.com:seaStarLxy/ECommerceSystem-Microservices.git

# 若已克隆但缺少子模块
git submodule update --init --recursive

### 2. 环境依赖安装
# 本项目推荐使用 Ubuntu 24.04 环境。

#### 基础工具链
```bash
sudo apt update
sudo apt install -y build-essential pkg-config ninja-build zip unzip tar libssl-dev curl gdb bison flex autoconf automake libtool git net-tools
```

#### C++ 依赖管理 (vcpkg)
使用 vcpkg 管理所有 C++ 第三方库。

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh

# 配置环境变量
echo "" >> ~/.bashrc
echo "# vcpkg config" >> ~/.bashrc
echo "export VCPKG_ROOT=~/vcpkg" >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### 3. 编译与构建
配置 CMake 时需指定 vcpkg 工具链：

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

### 4. 服务网关部署 (Envoy)
Envoy 负责将前端 HTTP 请求转发为后端 gRPC 调用。

```bash
# 1. 安装 Envoy (Debian/Ubuntu)
wget -O- [https://apt.envoyproxy.io/signing.key](https://apt.envoyproxy.io/signing.key) | sudo gpg --dearmor -o /etc/apt/keyrings/envoy-keyring.gpg
echo "deb [signed-by=/etc/apt/keyrings/envoy-keyring.gpg] [https://apt.envoyproxy.io](https://apt.envoyproxy.io) bookworm main" | sudo tee /etc/apt/sources.list.d/envoy.list
sudo apt update && sudo apt install envoy

# 检查安装
envoy --version

# 2. 生成 proto.pb (供 Envoy 进行协议转换使用)
# 注意：路径需根据实际环境调整
# 假设当前位于项目根目录
./vcpkg_installed/x64-linux/tools/protobuf/protoc \
  -I ./IDL \
  -I ./third_party/googleapis \
  --include_imports \
  --descriptor_set_out=./apiGateway/proto.pb \
  ./IDL/UserService/v1/user_service.proto

# 3. 启动 Envoy
envoy -c ./apiGateway/envoy.yaml -l info
```

### 5. 基础中间件
请确保已安装并启动以下服务：
1.  **PostgreSQL**: 用于数据持久化
2.  **Redis**: 用于状态管理与缓存
> 本项目使用云原生数据库
---

## 🧭 开发者指南

为保证代码质量与协作效率，项目遵循以下规范。

### 代码命名规范
| 类型 | 风格 | 示例 |
|:---|:---|:---|
| **类/结构体** | PascalCase | `UserSession`, `OrderManager` |
| **函数/方法** | PascalCase | `GetUserInfo()`, `CalculatePrice()` |
| **文件/目录** | snake_case | `user_service.h`, `async_logger.cpp` |
| **变量/参数** | snake_case | `user_id`, `request_timeout` |
| **成员变量** | snake_case + `_` | `db_connection_`, `mutex_` |
| **命名空间** | snake_case | `photon_commerce` |

### Git 工作流
本项目采用 Feature Branch 工作流。

1.  **同步主分支**: `git checkout main && git pull`
2.  **创建分支**:
    * 新功能: `git checkout -b feature/your-feature-name`
    * 修复 Bug: `git checkout -b bugfix/your-fix-name`
    * 重构: `git checkout -b refactor/your-refactor-name`
    * 性能优化: `git checkout -b perf/optimize-connection-pool`
    * 更新文档: `git checkout -b docs/update-readme`
    * 修改构建/配置: `git checkout -b chore/update-cmake`
3.  **提交代码**:
    ```bash
    git add .
    git commit -s -m "feat: implement jwt authentication logic"
    ```
4.  **推送与合并**:
    ```bash
    git push -u origin feature/your-feature-name
    # 随后在 GitHub 发起 Pull Request (PR) 并等待 CI 检查通过
    ```

---

## ⚖️ License

本项目采用 [MIT License](./LICENSE) 许可证。