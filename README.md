# 🐧 Linux Command Pro

<div align="center">

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/yourusername/The_Linux_Command_Pro)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/yourusername/The_Linux_Command_Pro/releases)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://github.com/yourusername/The_Linux_Command_Pro)
[![CMake](https://img.shields.io/badge/CMake-3.10+-blue.svg)](https://cmake.org/)

**一个增强版的Linux命令集合，在保持原有命令功能的基础上，提供了更好的用户体验和视觉效果。**

[![Demo](https://img.shields.io/badge/📖-查看演示-green.svg)](docs/usage.md)
[![安装指南](https://img.shields.io/badge/🔧-安装指南-orange.svg)](docs/installation.md)
[![功能特性](https://img.shields.io/badge/✨-功能特性-purple.svg)](docs/features.md)

</div>

## 🚀 快速开始

### 📦 安装
```bash
# 克隆项目
git clone https://github.com/yourusername/The_Linux_Command_Pro.git
cd The_Linux_Command_Pro

# 构建项目
./build.sh

# 安装到系统（可选）
sudo make install
```

### 🎯 快速体验
```bash
# 列出文件（带图标和颜色）
pls

# 显示文件内容（带语法高亮）
pcat main.c

# 搜索文件
pfind . -name "*.c"

# 搜索文本
pgrep "hello" file.txt

# 监控进程
ptop

# 分析磁盘使用
pdu
```

## 🎯 命令概览

<div align="center">

| 命令 | 功能 | 特色 | 状态 |
|------|------|------|------|
| **pls** | 文件列表 | 🎨 彩色输出 + 文件图标 | ✅ 完成 |
| **pcat** | 文件显示 | 🌈 语法高亮 + 行号 | ✅ 完成 |
| **pfind** | 文件搜索 | 🔍 智能搜索 + 美观输出 | ✅ 完成 |
| **pgrep** | 文本搜索 | 🎨 高亮匹配 + 统计信息 | ✅ 完成 |
| **ptop** | 进程监控 | 📊 实时监控 + 彩色显示 | 🚧 开发中 |
| **pdu** | 磁盘使用 | 📈 图形化显示 + 进度条 | 🚧 开发中 |
| **pps** | 进程列表 | 🌈 彩色进程 + 详细信息 | 🚧 开发中 |
| **pkill** | 进程管理 | ⚡ 智能终止 + 交互确认 | 📋 计划中 |
| **pfree** | 内存监控 | 🧠 内存可视化 + 实时监控 | 📋 计划中 |
| **pwho** | 用户信息 | 👥 用户登录 + 状态显示 | 📋 计划中 |
| **puptime** | 系统状态 | ⏰ 运行时间 + 负载监控 | 📋 计划中 |
| **pawk** | 文本处理 | 🔧 面向对象 + 高级功能 | 📋 计划中 |
| **psed** | 流编辑 | ✏️ 面向对象 + 语法高亮 | 📋 计划中 |
| **pnetstat** | 网络连接 | 🌐 连接可视化 + 统计信息 | 📋 计划中 |
| **pmount** | 挂载管理 | 💾 挂载点可视化 + 使用率 | 📋 计划中 |

</div>

> 📖 [查看详细使用教程](docs/usage.md) | 🔧 [安装指南](docs/installation.md) | ✨ [功能特性](docs/features.md)

## 🛠️ 技术栈

<div align="center">

| 技术 | 版本 | 说明 |
|------|------|------|
| **C语言** | C99 | 主要开发语言 |
| **构建系统** | CMake 3.10+ | 跨平台构建 |
| **依赖** | 标准C库 | 无外部依赖 |
| **平台** | Linux | 支持WSL2 |
| **编译器** | GCC/Clang | 支持C99标准 |

</div>


## 🎨 预览

### 📁 pls - 文件列表
```bash
$ pls
📁 目录内容 (5 个文件/目录)
====================
📁 📁 文档
📄 📄 README.md
⚡ ⚡ main.c
💻 💻 script.py
📦 📦 archive.zip
```

### 📄 pcat - 文件显示
```bash
$ pcat -n main.c
文件: main.c
====================
   1 | #include <stdio.h>
   2 | int main() {
   3 |     printf("Hello World\n");
   4 |     return 0;
   5 | }
```

### 🔍 pfind - 文件搜索
```bash
$ pfind . -name "*.c"
搜索结果 (3 个匹配项)
====================
⚡ ./src/main.c
⚡ ./src/utils.c
⚡ ./test/test.c
```

### 🔎 pgrep - 文本搜索
```bash
$ pgrep -n "printf" main.c
搜索结果 (1 个匹配项)
====================
   3:     printf("Hello World\n");
```

## 📁 项目结构

```
The_Linux_Command_Pro/
├── 📄 README.md              # 项目说明
├── 🔧 build.sh               # 构建脚本
├── 📋 CMakeLists.txt         # 主CMake配置
├── 📁 docs/                  # 文档目录
│   ├── 📖 usage.md           # 使用教程
│   ├── 🔧 installation.md    # 安装指南
│   └── ✨ features.md        # 功能特性
├── 📁 include/               # 头文件
│   └── 📄 common.h           # 公共头文件
├── 📁 src/                   # 源代码
│   └── 📄 common.c           # 公共实现
├── 📁 pls/                   # pls命令
│   ├── 📋 CMakeLists.txt
│   └── 📄 pls.c
├── 📁 pcat/                  # pcat命令
│   ├── 📋 CMakeLists.txt
│   └── 📄 pcat.c
├── 📁 pfind/                 # pfind命令
│   ├── 📋 CMakeLists.txt
│   └── 📄 pfind.c
├── 📁 pgrep/                 # pgrep命令
│   ├── 📋 CMakeLists.txt
│   └── 📄 pgrep.c
├── 📁 ptop/                  # ptop命令
│   ├── 📋 CMakeLists.txt
│   └── 📄 ptop.c
├── 📁 pdu/                   # pdu命令
│   ├── 📋 CMakeLists.txt
│   └── 📄 pdu.c
└── 📁 pps/                   # pps命令
    ├── 📋 CMakeLists.txt
    └── 📄 pps.c
```

## 🤝 贡献指南

我们欢迎各种形式的贡献！这个项目需要您的帮助来变得更好。

### 🚀 如何贡献
1. **Fork** 这个项目
2. 创建你的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交你的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 **Pull Request**

### 🎯 贡献类型
- 🐛 **报告Bug** - 发现问题请及时反馈
- ✨ **新功能建议** - 提出新的命令或功能
- 📝 **文档改进** - 完善使用说明和API文档
- 🎨 **代码优化** - 提升代码质量和性能
- 🧪 **测试用例** - 添加单元测试和集成测试
- 🌍 **国际化** - 添加多语言支持

### 📋 开发规范
- 遵循C99标准
- 使用4空格缩进
- 添加必要的注释
- 保持代码风格一致

## 📊 开发状态

### ✅ 已完成
- [x] **pls** 命令实现
- [x] **pcat** 命令实现
- [x] **pfind** 命令实现
- [x] **pgrep** 命令实现
- [x] CMake 构建系统
- [x] 基础文档

### 🚧 进行中
- [ ] **ptop** 进程监控命令
- [ ] **pdu** 磁盘使用分析命令
- [ ] **pps** 进程列表命令

### 📋 计划中
- [ ] 单元测试框架
- [ ] 性能测试套件
- [ ] 包管理器支持 (apt, yum, pacman)
- [ ] CI/CD 流水线
- [ ] 更多命令实现
- [ ] 配置文件支持
- [ ] 插件系统

## 📄 许可证

本项目采用 **MIT 许可证** - 查看 [LICENSE](LICENSE) 文件了解详情。


