# 代码打包

import os

# 设定要扫描的文件后缀（根据你的C++项目定制）
TARGET_EXTENSIONS = {
    '.cpp', '.c', '.cc', '.h', '.hpp',
    '.proto', '.sql', '.yaml', '.yml',
    '.txt', '.md', '.dockerfile'
}

# 设定必须要忽略的目录（防止文件过大炸掉）
IGNORE_DIRS = {
    '.git', '.idea', 'cmake-build-debug', 'cmake-build-debug-', 'cmake-build-release', 'cmake-build-release',
    'build', 'out', 'bin', 'third_party', 'vcpkg_installed', 'external', '.vscode', '.venv',
    '__pycache__', 'logs', 'generated'
}

# 特殊包含的文件名
INCLUDE_FILES = {'CMakeLists.txt', 'Dockerfile', 'docker-compose.yml'}

OUTPUT_FILE = 'project_context.txt'

def is_text_file(filename):
    return any(filename.endswith(ext) for ext in TARGET_EXTENSIONS) or filename in INCLUDE_FILES

def pack_project():
    with open(OUTPUT_FILE, 'w', encoding='utf-8') as outfile:
        # 提示词
        outfile.write("Below is the entire codebase context. Please analyze it based on the user's request.\n\n")

        for root, dirs, files in os.walk('.'):
            # 修改 dirs 列表以原地过滤需要忽略的目录
            dirs[:] = [d for d in dirs if d not in IGNORE_DIRS]

            for file in files:
                if is_text_file(file):
                    file_path = os.path.join(root, file)
                    # 跳过脚本自己
                    if file == 'packer.py' or file == OUTPUT_FILE:
                        continue

                    try:
                        with open(file_path, 'r', encoding='utf-8') as infile:
                            content = infile.read()
                            # 格式化输出：文件名 + 代码块
                            outfile.write(f"='='=' File: {file_path} '='='='\n")
                            outfile.write("```\n")
                            outfile.write(content)
                            outfile.write("\n```\n\n")
                            print(f"Packed: {file_path}")
                    except Exception as e:
                        print(f"Skipped {file_path} due to error: {e}")

    print(f"\n所有代码已打包到: {os.path.abspath(OUTPUT_FILE)}")

if __name__ == '__main__':
    pack_project()