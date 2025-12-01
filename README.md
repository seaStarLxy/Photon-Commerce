# Photon-Commerce

[**🇨🇳 中文文档**](./README_CN.md) | [**🇺🇸 English**](./README.md)


![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![C++ Version](https://img.shields.io/badge/C++-23-blue)
![Architecture](https://img.shields.io/badge/Architecture-Microservices-orange)
![Communication](https://img.shields.io/badge/gRPC-Asynchronous-red)
![Gateway](https://img.shields.io/badge/Envoy-Proxy-purple)
![License](https://img.shields.io/badge/License-MIT-green.svg)

**Photon-Commerce** is a high-performance, cloud-native distributed e-commerce platform engine built on **C++23** and **gRPC**.

This project aims to explore extreme backend performance and architectural design, integrating advanced concepts such as **Coroutines**, **Zero-Cost Abstractions**, **Lock-Free Programming**, and **Domain-Driven Design**.

---



## 🚀 Core Features

### ⚡️️ Extreme Performance & Asynchronous Architecture

- **End-to-End Asynchrony**: Achieves low latency and high concurrency via the **gRPC Async Model** and **C++20 Coroutines**. Wraps `libpq` for fully non-blocking I/O.
- **Strand Lock-Free Model**: Utilizes `Boost.Asio Strand` to serialize DB (MPMC pool) and Redis operations. Eliminates mutex contention and ensures thread safety without explicit locking.
- **CallData Compile-Time Polymorphism**: Features a 3-layer architecture ("Type-Erased Base + CRTP + Concepts") to enforce static polymorphism, achieving **zero virtual function overhead**.
- **Object Pooling**: Implements a generic `Manager` pool to reuse `CallData` instances, significantly reducing memory fragmentation from frequent allocations.
- **Caching & Reliability**: Integrates Redis **Cache-Aside** pattern and replaces runtime exceptions with `C++23 std::expected`, balancing code elegance with peak performance.
- **Zero-Cost Abstractions**: Leverages `Boost.DI` for compile-time Dependency Injection (IoC), ensuring modularity with **no runtime performance penalty**.

---

## 💪 Performance Showcase

### Performance Report Summary
| Test Type           | Test Objective                         | Method                          | Result Summary        |
|:--------------------|:---------------------------------------|:--------------------------------|:----------------------|
| **Micro-Benchmark** | Test engine's pure computational limit | Mock Data (No Redis)            | **38k QPS**           |
| **Benchmark**       | Test real-world scenario performance   | Read/Write Real Redis           | **23k ~ 25k QPS**     |
| **Long-Run Test**   | Test system robustness                 | Continuous run (100M+ requests) | **1h 41min, 0 Error** |

> Both Micro-Benchmark and Benchmark used two load generators simultaneously.
<details>
<summary>👉 <b>Click to expand: Micro-Benchmark Screenshot</b></summary>

![perf_micro_benchmark](docs/images/common/perf_micro_benchmark.png)
</details>


<details>
<summary>👉 <b>Click to expand: Benchmark Screenshot</b></summary>

![perf_benchmark](docs/images/common/perf_benchmark.png)
</details>


<details>
<summary>👉 <b>Click to expand: Long-Run Test Screenshot</b></summary>

![perf_long_run](docs/images/common/perf_long_run.png)
</details>



### 🛠️ Test Environment
To simulate real-world production constraints, this test did **not** use top-tier hardware. Instead, it aimed to explore architectural limits within a constrained cloud-native environment.

| Role                  | Deployment Platform | Instance Spec & Config                   | Note                                                                        |
|:----------------------|:--------------------|:-----------------------------------------|:----------------------------------------------------------------------------|
| **Server (App)**      | AWS EC2             | **`t3.xlarge`** (4 vCPU, 16GB RAM)       | Core Service (Ubuntu 24.04)                                                 |
| **Redis (Cache)**     | AWS ElastiCache     | **`cache.t3.micro`** (2 vCPU, 0.5GB RAM) | **System Bottleneck**, burstable instance limits sustained high concurrency |
| **PostgreSQL (DB)**   | AWS RDS             | **`db.t3.micro`** (2 vCPU, 1GB RAM)      | Low-spec persistence layer, used for cold data only                         |
| **Client 1 (Stress)** | AWS EC2             | **`c5.xlarge`** (4 vCPU, 8GB RAM)        | Dedicated load generator running `ghz`                                      |
| **Client 2 (Stress)** | AWS EC2             | **`c5.2xlarge`** (8 vCPU, 16GB RAM)      | Dedicated load generator running `ghz`                                      |
| **Network**           | AWS VPC             | Intranet (Same Availability Zone)        | Extremely low latency (< 1ms), excluding public network interference        |

## 📐 System Architecture & Core Design

### 1. Full-Link Backend Architecture (Envoy + gRPC + Redis + PostgreSQL)
![System Architecture](docs/images/en/system_architecture_en.png)

### 2. Dual Thread Pool Cooperative Async I/O Workflow
<details>
<summary>👉 <b>Click to expand details</b></summary>

![gRPC Async Design](docs/images/en/grpc_async_workflow_en.png)
</details>


### 3. CallData Template Implementation & Request State Flow
<details>
<summary>👉 <b>Click to expand details</b></summary>

![CallData Design](docs/images/en/call_data_impl_en.png)
</details>


### 4. Strand-Based Redis Request Serialization
<details>
<summary>👉 <b>Click to expand details</b></summary>

![Redis Strand](docs/images/en/redis_strand_en.png)
</details>

### 5. Lock-Free DB Connection Pool Design (Strand + Channel)
<details>
<summary>👉 <b>Click to expand details</b></summary>

![DB Pool Strand](docs/images/en/db_pool_strand_en.png)
</details>


### 6. Core Business Logic Flowchart
<details>
<summary>👉 <b>Click to expand details</b></summary>

![Core Business](docs/images/en/core_business_en.png)
</details>

---

## ⚙️ Tech Stack Overview

| Dimension       | Selection           | Core Considerations                           |
|:----------------|:--------------------|:----------------------------------------------|
| **Language**    | **C++23**           | Coroutines, Expected, Boost                   |
| **Protocol**    | **gRPC & Protobuf** | High-performance RPC, Asynchronous processing |
| **API Gateway** | **Envoy Proxy**     | Traffic management, REST to gRPC transcoding  |
| **Storage**     | **PostgreSQL**      | Relational data persistence                   |
| **Cache**       | **Redis**           | Hot data caching                              |
| **Dependency**  | **vcpkg & CMake**   | Modern C++ package management & build system  |
| **Container**   | **Docker**          | One-click deployment with docker-compose      |

### Key Libraries
| Library      | Purpose              | Description                                                                               |
|:-------------|:---------------------|:------------------------------------------------------------------------------------------|
| `googleapis` | API Transcoding      | Support for HTTP/JSON to gRPC mapping                                                     |
| `Boost`      | Core Components      | Includes `Boost.Asio` (Network/Coroutine scheduling) & `Boost.Redis` (Async Redis client) |
| `Boost.DI`   | Dependency Injection | Modern C++ compile-time dependency injection (Zero-overhead IoC)                          |
| `libpq`      | Database Driver      | PostgreSQL underlying C interface, encapsulated for asynchrony                            |
| `spdlog`     | Logging              | Very fast C++ logging library                                                             |
| `cryptopp`   | Security/Crypto      | Password hashing and encryption/decryption                                                |
| `jwt-cpp`    | Authentication       | JSON Web Token                                                                            |
| `yaml-cpp`   | Configuration        | YAML configuration file parsing                                                           |

---

## ▶️ Quick Start

> This guide helps you quickly set up a development environment locally.

### 1. Get the Source Code
**Note**: You must use the `--recursive` flag to include all submodules.

```bash
# Clone for the first time
git clone --recursive git@github.com:seaStarLxy/Photon-Commerce.git

# If cloned without submodules
git submodule update --init --recursive
```

### 2. Install Dependencies
This project recommends using Ubuntu 24.04.

#### Basic Toolchain
```bash
sudo apt update
sudo apt install -y build-essential pkg-config ninja-build zip unzip tar libssl-dev curl gdb bison flex autoconf automake libtool git net-tools
```

#### C++ Dependency Management (vcpkg)
Use vcpkg to manage all third-party C++ libraries.

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh

# Configure environment variables
echo "" >> ~/.bashrc
echo "# vcpkg config" >> ~/.bashrc
echo "export VCPKG_ROOT=~/vcpkg" >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### 3. Compile & Build
Configure CMake with the vcpkg toolchain file.

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

### 4. Deploy Service Gateway (Envoy)
Envoy is responsible for transcoding frontend HTTP requests into backend gRPC calls.

```bash
# 1. Install Envoy (Debian/Ubuntu)
wget -O- https://apt.envoyproxy.io/signing.key | sudo gpg --dearmor -o /etc/apt/keyrings/envoy-keyring.gpg
echo "deb [signed-by=/etc/apt/keyrings/envoy-keyring.gpg] https://apt.envoyproxy.io bookworm main" | sudo tee /etc/apt/sources.list.d/envoy.list
sudo apt update && sudo apt install envoy

# Check installation
envoy --version

# 2. Generate proto.pb (Used by Envoy for protocol transcoding)
# Note: Adjust paths according to your actual environment
# Assuming current directory is project root
./UserService/cmake-build-debug/vcpkg_installed/x64-linux/tools/protobuf/protoc \
  -I ./IDL \
  -I ./third_party/googleapis \
  --include_imports \
  --descriptor_set_out=./apiGateway/proto.pb \
  ./IDL/UserService/v1/user_service.proto

# 3. Start Envoy
envoy -c ./apiGateway/envoy.yaml -l info
```

### 5. Infrastructure
Please ensure the following services are installed and running:
1.  **PostgreSQL**: For data persistence
2.  **Redis**: For state management and caching
> This project utilizes cloud-native databases.
---

## 🧭 Developer Guide

To ensure code quality and collaboration efficiency, the project follows these standards.

### Naming Conventions
| Type                | Style            | Example                              |
|:--------------------|:-----------------|:-------------------------------------|
| **Class/Struct**    | PascalCase       | `UserSession`, `OrderManager`        |
| **Function/Method** | PascalCase       | `GetUserInfo()`, `CalculatePrice()`  |
| **File/Dir**        | snake_case       | `user_service.h`, `async_logger.cpp` |
| **Var/Param**       | snake_case       | `user_id`, `request_timeout`         |
| **Member Var**      | snake_case + `_` | `db_connection_`, `mutex_`           |
| **Namespace**       | snake_case       | `photon_commerce`                    |

### Git Workflow
This project adopts the Feature Branch workflow.

1.  **Sync Main Branch**: `git checkout main && git pull`
2.  **Create Branch**:
    * New Feature: `git checkout -b feature/your-feature-name`
    * Bug Fix: `git checkout -b bugfix/your-fix-name`
    * Refactor: `git checkout -b refactor/your-refactor-name`
    * Performance: `git checkout -b perf/optimize-connection-pool`
    * Documentation: `git checkout -b docs/update-readme`
    * Chore: `git checkout -b chore/update-cmake`
3.  **Commit Code**:
    ```bash
    git add .
    git commit -s -m "feat: implement jwt authentication logic"
    ```
4.  **Push & Merge**:
    ```bash
    git push -u origin feature/your-feature-name
    # Then create a Pull Request (PR) on GitHub and wait for CI checks to pass
    ```
### ⚠️ Compiler Compatibility & Known Issues
This project makes extensive use of **C++20/23 Coroutines** and the **`std::expected`** error handling mechanism. During development, it was discovered that certain versions of **`GCC (especially GCC 13)`** have internal defects (Internal Compiler Error, ICE) when handling implicit conversions of coroutine return values.

#### 🔴 GCC "Internal Compiler Error" Issue
If you encounter a crash similar to the following during compilation:
```text
internal compiler error: in build_special_member_call, at cp/call.cc:11096
```
This is typically caused by multi-level implicit type conversions in a co_return statement (e.g., User -> std::optional<...> -> std::expected<...>) or binding temporary objects to reference parameters of a coroutine.

#### ✅ Solution:
1. Avoid Implicit Conversion: Explicitly construct the final return object before co_return.
2. Avoid Binding Temporary Variables to References: When calling co_await functions, avoid creating temporary objects (like std::string or std::vector) directly in the parameter list. Define local variables beforehand.


---

## ⚖️ License

This project is licensed under the [MIT License](./LICENSE).