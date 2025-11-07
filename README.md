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
8. 健壮性实现：服务降级和对象池
8. 封装 grpc 客户端调用为协程
9. saga模式: 保证微服务事务


## 技术细节

### C++依赖
|    依赖库     |       作用        |       备注       |
|:----------:|:---------------:|:--------------:|
| googleapis | 提供 REST-grpc 转化 |                |
|   boost    |   提供协程管理&相关特性   |  实验性方案具有时效性！   |
|   libpq    |    提供数据库同步接口    | postgresql 数据库 |
|   spdlog   |      日志管理       |                |
|   bcrypt   |      密码加密       |                |
|  uuid_v7   |     唯一id管理      |                |
|  boost_di  |     提供依赖注入      |                |
|  jwt-cpp   |     提供jwt认证     |                |

### 数据库连接

### 数据库连接池

### grpc 异步实现


## 项目部署

### 本项目包含 Git 子模块

- **首次克隆项目**
  > 第一次克隆本项目，务必使用 `--recursive` 参数同时克隆子模块。

    ```bash
    git clone --recursive <Project URL>
    ```

- **更新子模块**
  > 如果已经克隆了项目，但忘记添加 `--recursive` 参数，或者子模块目录为空，运行以下命令初始化并拉取子模块。

    ```bash
    git submodule update --init --recursive
    ```
  
## 项目开发

### 命名规则
- 类/结构体: PascalCase (大驼峰) - MyClass
- 函数/方法: PascalCase (大驼峰) - MyFunction
- 文件名: snake_case (蛇形) - my_file.h
- 变量名: snake_case (蛇形) - my_variable (成员变量加 _ 后缀)
- 命名空间: snake_case (蛇形) - my_namespace


### 分支命名规则
- feature: 开发新功能
- bugfix: 修复bug
- chore: 修改配置等
- refactor: 大规模重构


### 开发规范
1. 开发前check out main
2. 根据*分支命名规则*创建分支
3. 进行开发后add commit push
4. 若 CICD 不通过，amend commit，然后force-with-lease推送

```shell
# 从main开始创建新分支工作
git checkout main
git pull
git checkout -b {new_branch_name}
git add && git commit && git commit -u origin {new_branch_name}
# 发生修改
git commit --amend && git commit --force-with-lease origin {new_branch_name}
```