# ECommerce System

## 前言

项目始于2025年10月，基于 NUS-ISS-SE 课程，尽可能以企业级标准实现， 
针对所学内容进行实践。主要基于以下技术考量：
1. 如何构建**可扩展**解决方案
2. 如何部署服务、治理服务
3. 核心质量属性: **性能**、可扩展、安全

### 技术栈
|  类型  |     选型      |     备注     |
|:----:|:-----------:|:----------:|
|  语言  |    C++20    | boost & 协程 |
|  通信  | REST & grpc |   异步grpc   |
|  架构  |     微服务     |   部分云原生    |
|  网关  |    envoy    |            |
| 持久化  | Postgresql  |            |
|  缓存  |    Redis    |            |
|  部署  |     k8s     |    待实现     |
| 服务治理 |             |    待实现     |


### 技术要点
1. 编译 grpc，并打包开发环境 docker 镜像
2. 编写 cmake，一站式编译 Protobuf
3. 基于 CallData， 实现异步 grpc
4. 添加 envoy，实现 RESTful API 自动转换 grpc 请求
5. 基于 pqlib 实现异步数据库连接和连接池（封装协程）
6. 耦合监听线程和协程（协程启动）
7. 架构分层，基于领域驱动设计，实现充血 domain 对象
8. boost di 管理依赖注入实现控制反转
9. CallData 封装（3层继承&模版&内存极致优化&安全控制&CRTP&概念约束）
10. 对象池(Manager模版)复用CallData
11. main 优化
12. 封装 grpc 客户端调用为协程
13. saga模式: 保证微服务事务

待定事件（未提上日程）：对象池的动态伸缩（无锁队列）


## 技术细节

### C++依赖
|    依赖库     |       作用        |       备注       |
|:----------:|:---------------:|:--------------:|
| googleapis | 提供 REST-grpc 转化 |                |
|   boost    |   提供协程管理&相关特性   |  实验性方案具有时效性！   |
|   libpq    |    提供数据库同步接口    | postgresql 数据库 |
| moodycamel |     提供无锁队列      |     暂时未启用      |
|   spdlog   |      日志管理       |                |
|  cryptopp  |      密码加密       |                |
|  uuid_v7   |     唯一id管理      |                |
|  boost_di  |     提供依赖注入      |                |
|  jwt-cpp   |     提供jwt认证     |                |
|  yaml-cpp  |     管理配置文件      |                |


### 数据库连接

### 数据库连接池

### grpc 异步实现

### CallData 封装（3层继承&模版&内存极致优化&安全控制&CRTP&概念约束）
> 这一部分的设计技术颇为广泛，耗时最久，实现巨难，有时间记得先补充这里


## 开发环境部署

### 1. 源代码拉取
> 务必注意：本项目包含 Git 子模块
```bash
# 首次克隆项目，务必使用 `--recursive` 参数同时克隆子模块。
git clone --recursive <Project URL>
```

```bash
# 如果已经克隆了项目，但忘记添加 `--recursive` 参数，或者子模块目录为空，运行以下命令初始化并拉取子模块。
git submodule update --init --recursive
```

### 2. 编译环境部署
#### 安装基础工具
```bash
sudo apt update
sudo apt install -y build-essential pkg-config ninja-build zip unzip tar libssl-dev curl gdb  bison flex autoconf automake libtool git net-tools
```

#### 安装 vcpkg
```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
echo "" >> ~/.bashrc
echo "# vcpkg config" >> ~/.bashrc
echo "export VCPKG_ROOT=~/vcpkg" >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
source ~/.bashrc
```

#### cmake 参数
> -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake

### 3. envoy 部署
#### 安装
```shell
wget -O- https://apt.envoyproxy.io/signing.key | sudo gpg --dearmor -o /etc/apt/keyrings/envoy-keyring.gpg
echo "deb [signed-by=/etc/apt/keyrings/envoy-keyring.gpg] https://apt.envoyproxy.io bookworm main" | sudo tee /etc/apt/sources.list.d/envoy.list
sudo apt update
sudo apt install envoy
envoy --version
```

#### 启动

```shell
# 生成 proto.pb
~/ECommerceSystem-Microservices/UserService/cmake-build-debug/vcpkg_installed/x64-linux/tools/protobuf/protoc \
  -I ~/ECommerceSystem-Microservices/IDL \
  -I ~/ECommerceSystem-Microservices/third_party/googleapis \
  --include_imports \
  --descriptor_set_out=~/ECommerceSystem-Microservices/apiGateway/proto.pb \
  ~/ECommerceSystem-Microservices/IDL/UserService/v1/user_service.proto
```
```shell
# 前台运行 envoy
envoy -c ~/ECommerceSystem-Microservices/apiGateway/envoy.yaml -l info
```

### 4. 数据库安装
1. 持久化: Postgresql
2. 状态管理: Redis
  
## 项目开发

### 1. 命名规则
- 类/结构体: PascalCase (大驼峰) - MyClass
- 函数/方法: PascalCase (大驼峰) - MyFunction
- 文件名: snake_case (蛇形) - my_file.h
- 变量名: snake_case (蛇形) - my_variable (成员变量加 _ 后缀)
- 命名空间: snake_case (蛇形) - my_namespace


### 2. 分支命名规则
- feature: 开发新功能
- bugfix: 修复bug
- chore: 修改配置等
- refactor: 大规模重构


### 3. 开发规范
1. 开发前check out main
2. 根据*分支命名规则*创建分支
3. 进行开发后add commit push
4. CICD 不通过需重新 commit 后提交

```shell
# 从main开始创建新分支工作
git checkout main
git pull
git checkout -b {new_branch_name}
# 此时进行开发
# 开发结束后可能需要分支改名（切换到需要改名的分支）
git branch -m {new_branch_name}
git add .
git commit -s -m "{your_commit_message}"
git push -u origin {new_branch_name}
# 发生修改则重复 commit 和 push
```

