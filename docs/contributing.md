# 🤝 贡献指南

我们欢迎各种形式的贡献！这个项目需要您的帮助来变得更好。

## 🚀 如何贡献

1. **Fork** 这个项目
2. 创建你的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交你的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 **Pull Request**

## 🎯 贡献类型

- 🐛 **报告Bug** - 发现问题请及时反馈
- ✨ **新功能建议** - 提出新的命令或功能
- 📝 **文档改进** - 完善使用说明和API文档
- 🎨 **代码优化** - 提升代码质量和性能
- 🧪 **测试用例** - 添加单元测试和集成测试
- 🌍 **国际化** - 添加多语言支持

## 📋 开发规范

### 代码规范
- 遵循C99标准
- 使用4空格缩进
- 添加必要的注释
- 保持代码风格一致

### 提交规范
- 提交信息使用中文或英文
- 格式：`类型: 简短描述`
- 类型包括：`feat`（新功能）、`fix`（修复）、`docs`（文档）、`style`（格式）、`refactor`（重构）、`test`（测试）、`chore`（构建）

### 代码审查
- 所有代码变更需要通过 Pull Request
- 至少需要一名维护者审查通过
- 确保代码通过所有测试

## 🔧 开发环境设置

### 前置要求
- Linux 系统（推荐 Ubuntu 20.04+）
- GCC 9.0+ 或 Clang 10.0+
- CMake 3.16+
- Git

### 设置步骤
```bash
# 克隆你的 Fork
git clone https://github.com/yourusername/The_Linux_Command_Pro.git
cd The_Linux_Command_Pro

# 添加上游仓库
git remote add upstream https://github.com/original/The_Linux_Command_Pro.git

# 构建项目
./build.sh
```

## 📝 添加新命令

### 步骤
1. 在项目根目录创建新的命令目录（如 `pnewcmd/`）
2. 创建 `CMakeLists.txt` 和源文件
3. 在主 `CMakeLists.txt` 中添加子目录
4. 更新文档（README、usage.md 等）
5. 添加测试用例

### 模板
```c
// pnewcmd/pnewcmd.c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // 实现你的命令
    return 0;
}
```

## 🐛 报告问题

### Bug 报告模板
- **问题描述**：清晰描述问题
- **复现步骤**：详细说明如何复现
- **预期行为**：应该发生什么
- **实际行为**：实际发生了什么
- **环境信息**：操作系统、编译器版本等
- **附加信息**：日志、截图等

## ❓ 获取帮助

- 查看 [使用教程](usage.md)
- 查看 [安装指南](installation.md)
- 提交 Issue：https://github.com/yourusername/The_Linux_Command_Pro/issues
- 查看 [开发状态](development.md)

## 📄 许可证

贡献的代码将遵循项目的 MIT 许可证。

