# ECommerce System

本项目包含 Git 子模块

* **首次克隆项目**
  > 第一次克隆本项目，务必使用 `--recursive` 参数同时克隆子模块。

    ```bash
    git clone --recursive <项目URL>
    ```

* **更新子模块**
  > 如果已经克隆了项目，但忘记添加 `--recursive` 参数，或者子模块目录为空，运行以下命令初始化并拉取子模块。

    ```bash
    git submodule update --init --recursive
    ```
