# ECommerce System

> 仅用于学习 grpc

## 项目部署

### 本项目包含 Git 子模块

* **首次克隆项目**
  > 第一次克隆本项目，务必使用 `--recursive` 参数同时克隆子模块。

    ```bash
    git clone --recursive <Project URL>
    ```

* **更新子模块**
  > 如果已经克隆了项目，但忘记添加 `--recursive` 参数，或者子模块目录为空，运行以下命令初始化并拉取子模块。

    ```bash
    git submodule update --init --recursive
    ```

## 命名规则
- 类/结构体: PascalCase (大驼峰) - MyClass
- 函数/方法: PascalCase (大驼峰) - MyFunction
- 文件名: snake_case (蛇形) - my_file.h
- 变量名: snake_case (蛇形) - my_variable (成员变量加 _ 后缀)
- 命名空间: snake_case (蛇形) - my_namespace


## 上传规则
- feature: 开发新功能
- bugfix: 修复bug
- chore: 修改配置等
- refactor: 大规模重构
