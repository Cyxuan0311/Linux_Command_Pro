# 🤝 Contributing Guide

We welcome contributions of all kinds! This project needs your help to become better.

## 🚀 How to Contribute

1. **Fork** this project
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a **Pull Request**

## 🎯 Contribution Types

- 🐛 **Report Bugs** - Please report any issues you find
- ✨ **Feature Suggestions** - Propose new commands or features
- 📝 **Documentation Improvements** - Improve usage instructions and API documentation
- 🎨 **Code Optimization** - Improve code quality and performance
- 🧪 **Test Cases** - Add unit tests and integration tests
- 🌍 **Internationalization** - Add multi-language support

## 📋 Development Guidelines

### Code Standards
- Follow C99 standard
- Use 4-space indentation
- Add necessary comments
- Maintain consistent code style

### Commit Standards
- Commit messages in Chinese or English
- Format: `Type: Short description`
- Types include: `feat` (feature), `fix` (bug fix), `docs` (documentation), `style` (formatting), `refactor` (refactoring), `test` (testing), `chore` (build)

### Code Review
- All code changes must go through Pull Request
- Requires approval from at least one maintainer
- Ensure all tests pass

## 🔧 Development Environment Setup

### Prerequisites
- Linux system (Ubuntu 20.04+ recommended)
- GCC 9.0+ or Clang 10.0+
- CMake 3.16+
- Git

### Setup Steps
```bash
# Clone your Fork
git clone https://github.com/yourusername/The_Linux_Command_Pro.git
cd The_Linux_Command_Pro

# Add upstream repository
git remote add upstream https://github.com/original/The_Linux_Command_Pro.git

# Build project
./build.sh
```

## 📝 Adding New Commands

### Steps
1. Create new command directory in project root (e.g., `pnewcmd/`)
2. Create `CMakeLists.txt` and source file
3. Add subdirectory to main `CMakeLists.txt`
4. Update documentation (README, usage.md, etc.)
5. Add test cases

### Template
```c
// pnewcmd/pnewcmd.c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Implement your command
    return 0;
}
```

## 🐛 Reporting Issues

### Bug Report Template
- **Description**: Clear description of the issue
- **Reproduction Steps**: Detailed steps to reproduce
- **Expected Behavior**: What should happen
- **Actual Behavior**: What actually happened
- **Environment Info**: OS, compiler version, etc.
- **Additional Info**: Logs, screenshots, etc.

## ❓ Getting Help

- View [Usage Tutorial](usage.md)
- View [Installation Guide](installation.md)
- Submit Issue: https://github.com/yourusername/The_Linux_Command_Pro/issues
- View [Development Status](development.md)

## 📄 License

Contributed code will follow the project's MIT License.

