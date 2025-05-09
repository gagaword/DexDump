# DexDump - Android DEX文件分析工具

## 项目简介

DexDump是一个强大的Android DEX文件解析和分析工具，能够帮助开发者、安全研究人员和逆向工程师深入理解Android应用的结构和行为。本工具可以解析DEX文件的各个部分，包括头部信息、字符串池、类型描述符、原型信息、字段、方法和类定义等，并以清晰易读的格式展示这些信息。

## 功能特点

- **完整的DEX文件解析**：支持解析DEX文件的所有主要部分
- **模块化设计**：清晰的代码结构，易于扩展和维护
- **详细的信息展示**：以格式化的方式展示DEX文件的各个部分
- **命令行界面**：支持灵活的命令行选项，方便批量处理
- **错误处理**：健壮的错误处理机制，确保解析过程的稳定性
- **跨平台支持**：计划支持Windows、Linux和macOS

## 已实现的功能

- DEX头部信息解析与展示
- 字符串池解析与展示
- 类型描述符解析与展示
- 原型信息解析与展示
- 字段定义解析与展示
- 方法定义解析与展示
- 类定义解析与展示
- 访问标志解析与格式化
- LEB128编码解析

## 安装指南

### 系统要求
- C++17兼容的编译器
- CMake 3.10或更高版本
- 支持Windows平台（未来将支持更多平台）

### 编译步骤

1. 克隆仓库
```bash
git clone https://github.com/yourusername/DexDump.git
cd DexDump
```

2. 创建构建目录
```bash
mkdir build
cd build
```

3. 配置并构建项目
```bash
cmake ..
cmake --build .
```

4. 运行测试（可选）
```bash
ctest
```

## 使用方法

### 基本用法

```bash
DexDump [选项] <dex文件路径>
```

### 命令行选项

- `-h, --header`：显示DEX文件头部信息
- `-s, --strings`：显示字符串池内容
- `-t, --types`：显示类型描述符
- `-p, --protos`：显示方法原型信息
- `-f, --fields`：显示字段定义
- `-m, --methods`：显示方法定义
- `-c, --classes`：显示类定义
- `-a, --all`：显示所有信息（默认）
- `--help`：显示帮助信息

### 示例

显示DEX文件的所有信息：
```bash
DexDump classes.dex
```

仅显示头部信息和字符串池：
```bash
DexDump -h -s classes.dex
```

仅显示类定义：
```bash
DexDump -c classes.dex
```

## 项目结构

```
DexDump/
├── include/                # 头文件
│   ├── core/               # 核心工具类
│   ├── model/              # 数据模型
│   ├── parser/             # 解析器
│   └── formatter/          # 格式化器
├── src/                    # 源文件
├── tests/                  # 测试文件
├── resources/              # 资源文件
├── docs/                   # 文档
├── CMakeLists.txt          # CMake构建脚本
└── README.md               # 项目说明
```

## DEX文件格式简介

DEX（Dalvik Executable）是Android平台使用的一种字节码文件格式，由Java类文件编译而成。DEX文件包含了应用程序的所有代码和数据，包括：

- 字符串常量
- 类型描述符
- 方法原型
- 字段定义
- 方法定义
- 类定义
- 方法代码
- 调试信息

## 开发路线图

- [x] 基本DEX文件解析
- [x] 命令行界面
- [ ] 完善代码解析
- [ ] 完善调试信息解析
- [ ] 支持DEX版本差异
- [ ] 图形用户界面
- [ ] 导出功能
- [ ] 多平台支持

## 贡献指南

欢迎贡献代码、报告问题或提出建议！请参考以下步骤：

1. Fork本仓库
2. 创建你的特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交你的更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建一个Pull Request

## 问题反馈

如果您在使用过程中遇到任何问题，请[提交issue](https://github.com/yourusername/DexDump/issues)，我们将尽快回复。

## 参考资料

- [Android官方文档 - DEX文件格式](https://source.android.com/devices/tech/dalvik/dex-format)
- [Android Open Source Project](https://source.android.com/)

## 许可证

本项目采用MIT许可证 - 详见 [LICENSE](LICENSE) 文件

## 致谢

感谢所有为本项目做出贡献的开发者和研究人员。特别感谢Android开源社区提供的技术文档和资源。

---
*DexDump - 深入理解Android应用的强大工具*