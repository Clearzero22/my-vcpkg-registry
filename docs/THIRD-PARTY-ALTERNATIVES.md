# 第三方库替代方案

| 自制库 | 推荐替代 | 说明 |
|--------|----------|------|
| `string-utils` | **fmtlib** (`fmt`) | 字符串格式化、split/join/trim 全覆盖 |
| `file-utils` | **std::filesystem** (C++17) / **ghc::filesystem** | 标准库已有，无需自制 |
| `crc-hash` | **zlib** (CRC32) / **OpenSSL** (MD5) | 工业级，经过广泛测试 |
| `binary-serializer` | **nlohmann/json** + **msgpack** / **Cap'n Proto** / **Protocol Buffers** | 成熟的序列化方案，支持 schema 演进 |
| `object-pool` | **boost::pool** / **EASTL fixed_vector** | Boost 的靠谱实现 |
| `ini-parser` | **Boost.PropertyTree** / **inih** (C) | 简单轻量 |
| `simple-logger` | **spdlog** | 纯头文件、高性能、异步、可配置 |
| `thread-pool` | **BS::thread_pool** / **Taskflow** | 现代 C++ 线程池 |
| `uuid` | **Boost.UUID** / **crossguid** / **stduuid** | UUID v4 生成 |
| `base64` | **cppcodec** / **OpenSSL base64** | 轻量编码 |
| `json-config` | **nlohmann/json** | C++ JSON 事实标准 |
| `tcp-client` | **cpp-httplib** (已引入) / **Boost.Asio** / **libcurl** | 网络通信 |

## 封装成本分析

### 即开即用（无需封装）
这些库 API 设计良好，`find_package` + 直接调用即可：

| 库 | 使用方式 |
|------|----------|
| **fmtlib** | `fmt::format("{}", x)` |
| **nlohmann/json** | `json j = json::parse(s); j["key"] = val;` |
| **spdlog** | `spdlog::info("msg {}", x);` |
| **CLI11** | `app.add_option("--port", port);` |
| **toml++** | `toml::parse_file("config.toml")` |
| **simdjson** | `parser.iterate(json);` |
| **magic_enum** | `magic_enum::enum_name(e)` |
| **cpp-httplib** | `cli.Get("/path")` |
| **YAML-CPP** | `YAML::LoadFile("config.yaml")` |
| **cereal** | `ar(CEREAL_NVP(x), CEREAL_NVP(y));` |

### 需要薄封装（少量适配层，10-50 行）

| 库 | 需要封装的内容 |
|------|---------------|
| **zlib** | C API 包裹成 RAII 类（`crc32()` 单函数可直接用） |
| **OpenSSL MD5** | C API 包裹成 RAII，自动 EVP_MD_CTX 创建/销毁 |
| **Boost.Asio** | 通常需要自己封装 I/O 上下文和会话生命周期 |
| **libcurl** | C API + 回调，封装成 RAII 类处理 CURL* 生命周期 |
| **Boost.PropertyTree** | `ptree.get<T>("path")` 可直接用，但类型处理需小心 |
| **inih** | C 回调 API，需封装成 C++ RAII 解析器类 |
| **bshoshany/thread-pool** | 单头文件，可直接用；如需任务优先级可加薄封装 |
| **parallel-hashmap** | 基本是 `std::unordered_map` 替换，用就行；个别情况需处理 ABI |

### 需要较多封装（>50 行，或生命周期管理复杂）

| 库 | 原因 |
|------|------|
| **Protobuf** | 需要 `.proto` 文件 + `protoc` 代码生成 CMake 集成 |
| **Cap'n Proto** | 同上，需 schema 编译步骤 |
| **FlatBuffers** | 同上，需 `flatc` 编译步骤 |
| **Boost.Beast** | Asio 基础 + HTTP/WebSocket 会话机，复杂度较高 |
| **concurrencpp** | 需要 `runtime` 实例、执行器管理、协程集成 |
| **libuv** | C API + 事件循环集成，需要封装为 C++ 类 |
| **OpenSSL** | C API，上下文/证书/连接管理，封装工作量最大 |

### 不适合封装（直接使用意义更大）

| 库 | 原因 |
|------|------|
| **abseil** | 设计就是替代标准库，封装反而引入复杂性 |
| **range-v3** | 管道操作式 API，封装会破坏组合性 |
| **immer** | 函数式 API，封装会破坏不可变性保证 |
| **zstd/lz4** | C API 但接口稳定，`ZSTD_compress()` 单行调用 |

## 自制库 vs 第三方库成本对比

| 自制库 | 当前维护成本 | 替换为第三方 | 封装成本 | 总体收益 |
|--------|-------------|-------------|---------|---------|
| `string-utils` | 低（代码稳定） | fmtlib | 无 | 低 |
| `json-config` | 高（有 UB 和 bug） | nlohmann/json | 无 | **高** |
| `simple-logger` | 中（线程安全缺陷） | spdlog | 无 | **高** |
| `object-pool` | 高（有 UB） | boost::pool | 低 | **高** |
| `binary-serializer` | 低 | cereal | 低 | 中 |
| `tcp-client` | 中（Winsock 复杂度） | cpp-httplib | 无 | **高** |

> 结论：`json-config`、`simple-logger`、`object-pool`、`tcp-client` 替换为第三方库收益最大，几乎无需封装。

## 其他优秀 C++ 库

### 核心工具
| 库 | 领域 | 说明 |
|------|------|------|
| **range-v3** | 范围库 | C++20 ranges 的前身，功能更全 |
| **expected** (T.110) | 错误处理 | `std::expected` 提案实现，替代异常/错误码 |
| **outcome** (Boost) | 错误处理 | 函数式错误处理，`result<T>` 模式 |
| **visit_struct** | 反射 | 结构体字段遍历，编译期反射 |
| **magic_enum** | 枚举反射 | `enum` ↔ 字符串互转 |
| **nameof** | 名称反射 | 编译期获取变量/函数名 |
| **hedley** | 可移植宏 | 跨编译器 `#ifdef` 统一封装 |

### 命令行 & 配置
| 库 | 领域 | 说明 |
|------|------|------|
| **CLI11** | CLI 解析 | 纯头文件、现代 C++、自动生成帮助 |
| **args** | CLI 解析 | 更轻量级的参数解析 |
| **toml++** | TOML 解析 | 纯头文件、C++17 TOML 配置 |
| **YAML-CPP** | YAML 解析 | YAML 事实标准 |

### 数据 & 序列化
| 库 | 领域 | 说明 |
|------|------|------|
| **Protobuf** | 序列化 | Google 出品，带代码生成、schema 校验 |
| **FlatBuffers** | 序列化 | 零解析开销，适合游戏/移动端 |
| **Cap'n Proto** | 序列化 | 比 Protobuf 更快，无编码步骤 |
| **Msgpack** | 序列化 | 类似 JSON 但二进制、更紧凑 |
| **cereal** | 序列化 | 纯头文件，支持 JSON/XML/Binary |

### 网络 & HTTP
| 库 | 领域 | 说明 |
|------|------|------|
| **Boost.Beast** | HTTP/WebSocket | Boost 官方，底层 Asio，功能完整 |
| **ixwebsocket** | WebSocket | 支持 WebSocket、HTTP Server |
| **curl/url** | URL 解析 | cURL 团队出品，c++17 URL parser |
| **nghttp2** | HTTP/2 | 纯 C，HTTP/2 及 HPACK 实现 |

### 并发 & 异步
| 库 | 领域 | 说明 |
|------|------|------|
| **libdispatch** (GCD) | 并发 | 苹果开源，任务队列模型 |
| **concurrencpp** | 协程 | 基于 C++20 协程的任务运行时 |
| **libuv** | 事件循环 | Node.js 底层，跨平台异步 I/O |

### 压缩 & 编码
| 库 | 领域 | 说明 |
|------|------|------|
| **zstd** (Facebook) | 压缩 | 比 zlib 快、压缩比更高 |
| **lz4** | 压缩 | 极致解压速度 |
| **NanoID** | ID 生成 | UUID 替代，更短更安全 |

### 图形 & GUI
| 库 | 领域 | 说明 |
|------|------|------|
| **Dear ImGui** | 即时 GUI | 调试工具、编辑器内嵌 UI |
| **SDL2** | 多媒体 | 游戏/窗口/输入/音频抽象 |
| **SFML** | 多媒体 | 比 SDL2 更现代(C++) |

### 其他
| 库 | 领域 | 说明 |
|------|------|------|
| **abseil** (Google) | 基础库 | 补全标准库缺失部分 |
| **parallel-hashmap** | 哈希表 | 比 `std::unordered_map` 快 2-5x |
| **robin-hood-hashing** | 哈希表 | 极速开放寻址哈希 |
| **simdjson** | JSON 解析 | 每秒 GB 级解析性能 |
| **immer** | 不可变数据结构 | 函数式编程，持久化向量/映射 |
| **ctre** | 正则表达式 | 编译期正则，性能远超 `std::regex` |
