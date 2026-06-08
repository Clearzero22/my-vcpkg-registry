# 基础库架构设计文档

## 概述

本文档定义了一套轻量级 C++ 基础库，用于支撑日常开发中的通用需求。所有库采用 CMake 构建，支持跨平台，并打包为 vcpkg 端口发布到本私有仓库。

## 设计原则

1. **轻量无依赖** — 每个库仅依赖 C++17 标准库，不引入第三方依赖
2. **头文件优先** — 工具类库尽量设计为 header-only，减少链接开销
3. **单一职责** — 每个库只做一件事，做好一件事
4. **统一风格** — 一致的命名风格、错误处理和 API 设计
5. **简易测试** — 每个库提供简单的测试示例

## 库目录

| # | 库名 | 类型 | 说明 |
|---|------|------|------|
| 1 | simple-logger | 静态库 | 轻量日志库，支持级别、文件轮转、控制台输出 |
| 2 | ini-parser | header-only | INI 配置文件解析器 |
| 3 | string-utils | header-only | 字符串工具集（分割/合并/修剪/编码） |
| 4 | file-utils | header-only | 文件系统工具（读写/路径/遍历） |
| 5 | binary-serializer | header-only | 二进制序列化（支持基础类型和 STL 容器） |
| 6 | crc-hash | header-only | CRC32/MD5/XXHash 哈希算法 |
| 7 | object-pool | header-only | 对象池/内存池 |
| 8 | tcp-client | 静态库 | TCP Socket 客户端封装（Win32 + POSIX） |

## 详细设计

### 1. simple-logger

**类型**: 静态库（需全局状态管理）
**头文件**: `<simple-logger/logger.hpp>`
**功能**:
- 日志级别: DEBUG, INFO, WARN, ERROR, FATAL
- 输出目标: 控制台, 文件（支持轮转）, 同时输出
- 格式化: 时间戳 + 级别 + [标签] + 消息
- 线程安全: 内部加锁

**API**:
```cpp
namespace simple {
  enum class log_level { debug, info, warn, error, fatal };

  class logger {
  public:
    static logger& instance();
    void set_level(log_level lv);
    void add_console_sink();
    void add_file_sink(std::string_view path, std::size_t max_size = 5 * 1024 * 1024, int max_files = 3);
    void log(log_level lv, std::string_view tag, std::string_view msg);
  };
}

#define LOG_DEBUG(tag, msg)   simple::logger::instance().log(simple::log_level::debug, tag, msg)
#define LOG_INFO(tag, msg)    simple::logger::instance().log(simple::log_level::info, tag, msg)
#define LOG_WARN(tag, msg)    simple::logger::instance().log(simple::log_level::warn, tag, msg)
#define LOG_ERROR(tag, msg)   simple::logger::instance().log(simple::log_level::error, tag, msg)
#define LOG_FATAL(tag, msg)   simple::logger::instance().log(simple::log_level::fatal, tag, msg)
```

---

### 2. ini-parser

**类型**: header-only
**头文件**: `<ini-parser/ini_parser.hpp>`
**功能**:
- 解析 INI 文件（`[section]`, `key=value`, `; comment`）
- 支持 `[]` `""` 包裹的值
- 读写/修改/保存

**API**:
```cpp
namespace ini {
  class parser {
  public:
    bool load(std::string_view path);
    bool load_from_string(std::string_view content);
    bool save(std::string_view path);
    std::optional<std::string> get(std::string_view section, std::string_view key) const;
    template<typename T> std::optional<T> get_as(std::string_view section, std::string_view key) const;
    void set(std::string_view section, std::string_view key, std::string_view value);
    bool has_section(std::string_view section) const;
    bool has_key(std::string_view section, std::string_view key) const;
    std::vector<std::string> sections() const;
    std::vector<std::string> keys(std::string_view section) const;
  };
}
```

---

### 3. string-utils

**类型**: header-only
**头文件**: `<string-utils/string_utils.hpp>`
**功能**:

```cpp
namespace str {
  // 修剪
  std::string trim(std::string_view s);
  std::string trim_left(std::string_view s);
  std::string trim_right(std::string_view s);

  // 分割/合并
  std::vector<std::string> split(std::string_view s, std::string_view delimiter);
  std::string join(std::span<const std::string> parts, std::string_view delimiter);

  // 大小写转换
  std::string to_lower(std::string_view s);
  std::string to_upper(std::string_view s);

  // 编码转换
  std::string to_hex(std::span<const std::byte> data);
  std::vector<std::byte> from_hex(std::string_view hex);

  // 替换
  std::string replace(std::string_view s, std::string_view from, std::string_view to);
  bool replace_in_place(std::string& s, std::string_view from, std::string_view to);

  // 检查
  bool starts_with(std::string_view s, std::string_view prefix);
  bool ends_with(std::string_view s, std::string_view suffix);
  bool contains(std::string_view s, std::string_view sub);
}
```

---

### 4. file-utils

**类型**: header-only
**头文件**: `<file-utils/file_utils.hpp>`
**功能**:

```cpp
namespace file {
  // 读写
  std::optional<std::string> read_all(std::string_view path);
  bool write_all(std::string_view path, std::string_view content);
  bool append_all(std::string_view path, std::string_view content);

  // 路径
  std::string dirname(std::string_view path);
  std::string filename(std::string_view path);
  std::string stem(std::string_view path);
  std::string extension(std::string_view path);
  std::string join(std::string_view base, std::string_view name);

  // 文件系统
  bool exists(std::string_view path);
  bool is_file(std::string_view path);
  bool is_directory(std::string_view path);
  std::uintmax_t file_size(std::string_view path);
  bool create_directories(std::string_view path);
  bool copy_file(std::string_view from, std::string_view to, bool overwrite = false);
  bool remove(std::string_view path);
  bool remove_all(std::string_view path);
  bool rename(std::string_view from, std::string_view to);

  // 遍历
  std::vector<std::string> list_files(std::string_view dir);
  std::vector<std::string> list_directories(std::string_view dir);
  void walk(std::string_view dir, std::function<void(const std::string&)> callback, bool recursive = true);
}
```

---

### 5. binary-serializer

**类型**: header-only
**头文件**: `<binary-serializer/binary_serializer.hpp>`
**功能**:
- 序列化支持: 整数、浮点、字符串、`vector`、`map`、`pair`、`tuple`
- 小端序输出
- 流式接口

**API**:
```cpp
namespace bin {

  class writer {
  public:
    void write(const void* data, std::size_t size);
    template<typename T> void write(T value);
    template<typename T> void write(const std::vector<T>& vec);
    template<typename K, typename V> void write(const std::map<K, V>& map);
    template<typename... Args> void write(const std::tuple<Args...>& t);
    std::span<const std::byte> data() const;
    const std::vector<std::byte>& bytes() const;
    void clear();
  };

  class reader {
  public:
    reader(std::span<const std::byte> data);
    bool read(void* data, std::size_t size);
    template<typename T> std::optional<T> read();
    template<typename T> std::optional<std::vector<T>> read_vector();
    bool eof() const;
    std::size_t remain() const;
  };

}
```

---

### 6. crc-hash

**类型**: header-only
**头文件**: `<crc-hash/crc32.hpp>`, `<crc-hash/md5.hpp>`
**功能**:

```cpp
namespace hash {
  std::uint32_t crc32(std::span<const std::byte> data);
  std::uint32_t crc32(std::string_view str);

  struct md5_digest {
    std::uint8_t bytes[16];
    std::string to_string() const;
    bool operator==(const md5_digest&) const = default;
  };

  md5_digest md5(std::span<const std::byte> data);
  md5_digest md5(std::string_view str);

  // 文件哈希
  std::optional<std::uint32_t> crc32_file(std::string_view path);
  std::optional<md5_digest> md5_file(std::string_view path);
}
```

---

### 7. object-pool

**类型**: header-only
**头文件**: `<object-pool/object_pool.hpp>`
**功能**:
- 泛型对象池，支持任意类型
- 自动增长、复用
- 线程安全选项

**API**:
```cpp
namespace pool {
  template<typename T>
  class object_pool {
  public:
    explicit object_pool(std::size_t initial_size = 16, bool thread_safe = true);

    // 获取对象（如果池为空则创建新的）
    std::shared_ptr<T> acquire();

    // 归还对象（内部自动通过 shared_ptr 的 deleter 归还）
    // 或手动
    void release(T* obj);

    // 状态
    std::size_t available() const;
    std::size_t used() const;
    std::size_t total() const;

    // 预分配
    void reserve(std::size_t count);

    // 清空池
    void clear();
  };
}
```

---

### 8. tcp-client

**类型**: 静态库（平台相关实现）
**头文件**: `<tcp-client/tcp_client.hpp>`
**功能**:
- TCP Socket 客户端封装
- 支持阻塞/非阻塞模式
- Windows (WinSock2) + Linux (POSIX) 跨平台

**API**:
```cpp
namespace net {
  class tcp_client {
  public:
    tcp_client();
    ~tcp_client();

    bool connect(std::string_view host, std::uint16_t port, int timeout_ms = 5000);
    void disconnect();
    bool is_connected() const;

    int send(std::span<const std::byte> data);
    int send(std::string_view data);

    int receive(std::span<std::byte> buffer);
    std::optional<std::string> receive_until(std::string_view delimiter);

    void set_timeout(int ms);
    void set_nodelay(bool enable);

  private:
    // 平台相关实现
    #ifdef _WIN32
      SOCKET sock_;
    #else
      int sock_;
    #endif
  };
}
```

## 版本规划

| 版本 | 内容 |
|------|------|
| v1.0.0 | 7 个 header-only 库 + 2 个静态库 |
| v1.1.0 | 单元测试、CI 集成 |
| v1.2.0 | 性能优化、benchmark |

## 发布流程

1. 在 `ports/<lib>/source/` 中编写源码
2. 编写 `vcpkg.json` 和 `portfile.cmake`
3. 本地测试安装 `vcpkg install <lib> --overlay-ports=<path>/ports`
4. 更新 `versions/` 目录中的版本数据库
5. 提交 Git 仓库
