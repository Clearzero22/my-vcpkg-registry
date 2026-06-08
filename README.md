# 如何使用私有 vcpkg 仓库

## 📁 项目结构已创建

```
my-vcpkg-registry/
├── vcpkg.json                    # 注册表配置
├── ports/                        # 包定义目录
│   └── example-package/
│       ├── portfile.cmake       # 构建脚本
│       ├── vcpkg.json           # 包元数据
│       └── usage                # 使用说明
└── versions/                     # 版本数据库
    ├── version-data.json
    └── v1.0.0.json
```

## 🔧 注册私有仓库

### 方式 1: 环境变量（推荐）

```bash
# 添加到 ~/.bashrc 或 ~/.zshrc
export VCPKG_OVERLAY_PORTS="/c/Users/admin/my-vcpkg-registry/ports"

# 重新加载配置
source ~/.bashrc
```

### 方式 2: 命令行指定

```bash
vcpkg install example-package --overlay-ports=/c/Users/admin/my-vcpkg-registry/ports
```

### 方式 3: vcpkg-configuration.json

在项目根目录创建 `vcpkg-configuration.json`:

```json
{
  "registries": [
    {
      "kind": "filesystem",
      "baseline": "a1b2c3d4e5f6g7h8i9j0",
      "path": "/c/Users/admin/my-vcpkg-registry"
    }
  ],
  "default-registry": {
    "kind": "git",
    "baseline": "a1b2c3d4e5f6g7h8i9j0",
    "repository": "https://github.com/microsoft/vcpkg"
  }
}
```

## 📦 创建新包

### 使用 vcpkg 工具快速创建

```bash
# 创建新包（会自动生成模板）
vcpkg create my-new-library https://github.com/user/library

# 编辑包定义
vcpkg edit my-new-library
```

### 手动创建包结构

```bash
mkdir -p ~/my-vcpkg-registry/ports/my-new-library
cd ~/my-vcpkg-registry/ports/my-new-library
```

然后创建以下文件：

#### 1. vcpkg.json（包元数据）

```json
{
  "name": "my-new-library",
  "version": "2.0.0",
  "description": "My custom C++ library",
  "homepage": "https://github.com/yourusername/my-new-library",
  "license": "MIT",
  "dependencies": [
    "fmt",
    "spdlog"
  ],
  "features": {
    "json": {
      "description": "JSON support",
      "dependencies": ["nlohmann-json"]
    },
    "xml": {
      "description": "XML support",
      "dependencies": ["pugixml"]
    }
  }
}
```

#### 2. portfile.cmake（构建脚本）

```cmake
# 下载源码
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO yourusername/my-new-library
    REF "v${VERSION}"
    SHA512 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
    HEAD_REF main
)

# 配置和构建
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
)

vcpkg_cmake_build()
vcpkg_cmake_install()

# 导出 CMake 配置
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/my-new-library)

# 安装许可证
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
```

#### 3. usage（使用说明）

```
# My New Library

## Usage

```cmake
find_package(my-new-library CONFIG REQUIRED)
target_link_libraries(main PRIVATE my-new-library::my-new-library)
```

## Features

```bash
# Install with JSON support
vcpkg install my-new-library[json]

# Install with XML support
vcpkg install my-new-library[xml]

# Install with both
vcpkg install my-new-library[json,xml]
```
```

## 🔄 更新版本数据库

当创建新版本时，更新版本文件：

```bash
# 更新 versions/v2.0.0.json
cd ~/my-vcpkg-registry/versions
```

编辑 `v2.0.0.json`:

```json
{
  "default": {
    "my-new-library": {
      "baseline": "2.0.0",
      "versions": ["2.0.0", "1.0.0"]
    }
  }
}
```

## 🧪 测试你的包

```bash
# 测试包是否可构建
vcpkg install my-new-library --overlay-ports=/c/Users/admin/my-vcpkg-registry/ports

# 清理并重新安装
vcpkg remove my-new-library --recurse
vcpkg install my-new-library --overlay-ports=/c/Users/admin/my-vcpkg-registry/ports
```

## 🚀 发布到远程仓库

```bash
cd ~/my-vcpkg-registry
git remote add origin https://github.com/yourusername/my-vcpkg-registry.git
git push -u origin main
```

然后在 `vcpkg-configuration.json` 中使用：

```json
{
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/yourusername/my-vcpkg-registry.git",
      "baseline": "a1b2c3d4e5f6g7h8i9j0"
    }
  ]
}
```

## 📋 常用命令

```bash
# 查看已注册的 overlay ports
echo $VCPKG_OVERLAY_PORTS

# 搜索本地包
vcpkg search --overlay-ports=/c/Users/admin/my-vcpkg-registry/ports

# 查看包信息
vcpkg x-package-info my-new-library --overlay-ports=/c/Users/admin/my-vcpkg-registry/ports

# 查看依赖关系
vcpkg depend-info my-new-library --overlay-ports=/c/Users/admin/my-vcpkg-registry/ports
```

## 💡 最佳实践

1. **使用 SHA512 哈希**: 在 `vcpkg_from_github` 中始终指定 SHA512
2. **遵循 vcpkg 命名规范**: 小写字母、连字符、数字
3. **提供清晰的使用说明**: 在 `usage` 文件中包含完整示例
4. **设置合适的依赖**: 只声明真正需要的依赖
5. **支持多平台**: 在 `vcpkg.json` 中使用 `supports` 字段
6. **版本控制**: 将仓库提交到 Git 便于管理

---

现在你有了一个可工作的私有 vcpkg 仓库！可以开始创建自己的包了。需要我帮你创建特定的包或配置吗？