# Windows 环境下使用 vcpkg 私有仓库

## ✅ 仓库已创建

位置: `C:\Users\admin\my-vcpkg-registry`

---

## 🚀 方式 1: 使用项目配置文件（推荐）

在你的 CMake 项目根目录创建 `vcpkg-configuration.json`：

```json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg-configuration.schema.json",
  "default-registry": {
    "kind": "git",
    "repository": "https://github.com/microsoft/vcpkg",
    "baseline": "a1b2c3d4e5f6g7h8i9j0"
  },
  "registries": [
    {
      "kind": "filesystem",
      "path": "C:\\Users\\admin\\my-vcpkg-registry",
      "baseline": "a1b2c3d4e5f6g7h8i9j0"
    }
  ]
}
```

然后创建 `vcpkg.json`（项目依赖）：

```json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "name": "my-cmake-project",
  "version": "1.0.0",
  "dependencies": [
    "example-package",
    "fmt"
  ]
}
```

安装依赖：

```cmd
vcpkg install
```

---

## 🔧 方式 2: 设置环境变量

### 临时设置（当前命令行会话）

```cmd
REM 在 cmd.exe 中
set VCPKG_OVERLAY_PORTS=C:\Users\admin\my-vcpkg-registry\ports

REM 在 PowerShell 中
$env:VCPKG_OVERLAY_PORTS="C:\Users\admin\my-vcpkg-registry\ports"

REM 在 Git Bash 中
export VCPKG_OVERLAY_PORTS="/c/Users/admin/my-vcpkg-registry/ports"
```

### 永久设置（系统环境变量）

1. 按 `Win + R`，输入 `sysdm.cpl`
2. 点击 "高级" → "环境变量"
3. 在 "用户变量" 或 "系统变量" 中添加：

   **变量名**: `VCPKG_OVERLAY_PORTS`
   **变量值**: `C:\Users\admin\my-vcpkg-registry\ports`

4. 点击 "确定" 并重启终端

### 快速配置脚本

运行提供的配置脚本：

```cmd
C:\Users\admin\my-vcpkg-registry\setup-windows.bat
```

---

## 📦 方式 3: 命令行指定（一次性）

```cmd
REM 在 cmd.exe 中
vcpkg install example-package --overlay-ports=C:\Users\admin\my-vcpkg-registry\ports

REM 在 PowerShell 中
vcpkg install example-package --overlay-ports=C:\Users\admin\my-vcpkg-registry\ports

REM 在 Git Bash 中
vcpkg install example-package --overlay-ports=/c/Users/admin/my-vcpkg-registry/ports
```

---

## 🎯 常用命令

### 搜索包

```cmd
vcpkg search example-package
```

输出：
```
example-package          1.0.0            A custom example package for demonstration
example-package[feature1]                 Enable optional feature 1
example-package[feature2]                 Enable optional feature 2
```

### 安装包

```cmd
# 基本安装
vcpkg install example-package

# 指定平台和架构
vcpkg install example-package:x64-windows
vcpkg install example-package:x86-windows

# 启用特性
vcpkg install example-package[feature1]:x64-windows

# 多个包
vcpkg install example-package fmt nlohmann-json
```

### 查看包信息

```cmd
# 查看已安装包
vcpkg list

# 查看包详细信息
vcpkg x-package-info example-package

# 查看依赖关系
vcpkg depend-info example-package

# 检查包支持情况
vcpkg x-check-support example-package
```

### 卸载包

```cmd
# 基本卸载
vcpkg remove example-package

# 递归卸载（包含依赖）
vcpkg remove example-package --recurse

# 卸载所有包
vcpkg remove --outdated
```

### 更新包

```cmd
# 查看可更新包
vcpkg update

# 更新所有过时包
vcpkg upgrade

# 更新指定包
vcpkg upgrade example-package
```

---

## 🔨 项目集成示例

### 创建 CMake 项目

**CMakeLists.txt**:

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 vcpkg 安装的包
find_package(example-package CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# 创建可执行文件
add_executable(main main.cpp)

# 链接库
target_link_libraries(main PRIVATE
    example-package::example-package
    fmt::fmt
)
```

**main.cpp**:

```cpp
#include <iostream>
#include <fmt/format.h>

// 假设 example-package 提供了这个头文件
#include <example/example.hpp>

int main() {
    std::cout << fmt::format("Hello from vcpkg package!") << std::endl;
    return 0;
}
```

### 构建项目

```cmd
REM 创建构建目录
mkdir build
cd build

REM 配置项目（使用 vcpkg 工具链）
cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE=D:\code_tools\c_plus_plus_tools\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows

REM 构建
cmake --build . --config Release

REM 运行
Release\main.exe
```

---

## 🆕 创建新包

### 使用 vcpkg 工具

```cmd
REM 创建新包模板
vcpkg create my-custom-library https://github.com/username/my-library

REM 编辑包定义
vcpkg edit my-custom-library
```

### 手动创建

```cmd
REM 创建包目录
mkdir C:\Users\admin\my-vcpkg-registry\ports\my-library
cd C:\Users\admin\my-vcpkg-registry\ports\my-library
```

创建 `vcpkg.json`:

```json
{
  "name": "my-library",
  "version": "1.0.0",
  "description": "My custom C++ library",
  "homepage": "https://github.com/yourusername/my-library",
  "license": "MIT",
  "dependencies": [
    "fmt"
  ],
  "features": {
    "json": {
      "description": "JSON support",
      "dependencies": ["nlohmann-json"]
    }
  }
}
```

创建 `portfile.cmake`:

```cmake
# 从 GitHub 下载源码
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO yourusername/my-library
    REF "v${VERSION}"
    SHA512 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
    HEAD_REF main
)

# 配置 CMake
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
)

# 构建
vcpkg_cmake_build()

# 安装
vcpkg_cmake_install()

# 导出 CMake 配置
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/my-library)

# 安装许可证
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
```

创建 `usage` 文件:

````cmake
# Usage

## CMake

```cmake
find_package(my-library CONFIG REQUIRED)
target_link_libraries(main PRIVATE my-library::my-library)
```

## Features

### JSON Support
```bash
vcpkg install my-library[json]:x64-windows
```
````

### 获取 SHA512 哈希

```cmd
REM 下载源码文件后计算哈希
certutil -hashfile downloaded.zip SHA512

REM 或使用 PowerShell
Get-FileHash -Path downloaded.zip -Algorithm SHA512
```

---

## 🧪 测试你的包

```cmd
REM 测试安装
vcpkg install my-library --overlay-ports=C:\Users\admin\my-vcpkg-registry\ports

REM 查看是否安装成功
vcpkg list | findstr my-library

REM 查看安装位置
vcpkg show my-library
```

---

## 🔍 常见问题

### 1. 找不到 vcpkg 命令

```cmd
REM 将 vcpkg 添加到 PATH
set PATH=%PATH%;D:\code_tools\c_plus_plus_tools\vcpkg\vcpkg

REM 或在系统环境变量中添加
PATH 变量添加: D:\code_tools\c_plus_plus_tools\vcpkg\vcpkg
```

### 2. CMake 找不到 vcpkg 安装的包

确保在配置 CMake 时指定工具链：

```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=D:\code_tools\c_plus_plus_tools\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### 3. 包搜索不到私有仓库的包

检查环境变量是否正确设置：

```cmd
REM cmd
echo %VCPKG_OVERLAY_PORTS%

REM PowerShell
echo $env:VCPKG_OVERLAY_PORTS

REM Git Bash
echo $VCPKG_OVERLAY_PORTS
```

### 4. 权限问题

以管理员身份运行命令提示符：

```cmd
# 右键点击 cmd.exe → "以管理员身份运行"
```

---

## 📚 有用的资源

- vcpkg 官方文档: https://learn.microsoft.com/zh-cn/vcpkg/
- vcpkg GitHub: https://github.com/microsoft/vcpkg
- CMake 与 vcpkg 集成: https://learn.microsoft.com/zh-cn/vcpkg/users/buildsystems/cmake

---

## 🎉 快速开始

1. **选择配置方式**：推荐使用 `vcpkg-configuration.json`
2. **测试搜索**：`vcpkg search example-package`
3. **安装包**：`vcpkg install example-package`
4. **集成项目**：使用 CMake 工具链构建项目

有任何问题随时问我！