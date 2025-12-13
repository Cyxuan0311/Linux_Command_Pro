# 🐧 Linux Command Pro

<div align="center">

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/yourusername/The_Linux_Command_Pro)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/yourusername/The_Linux_Command_Pro/releases)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://github.com/yourusername/The_Linux_Command_Pro)
[![CMake](https://img.shields.io/badge/CMake-3.10+-blue.svg)](https://cmake.org/)

**An enhanced Linux command collection that provides better user experience and visual effects while maintaining the original command functionality.**

[![中文](https://img.shields.io/badge/中文-简体中文-red.svg)](README.md) | [![Usage](https://img.shields.io/badge/📖-Usage-green.svg)](docs/usage.md) | [![Installation](https://img.shields.io/badge/🔧-Installation-orange.svg)](docs/installation.md) | [![Features](https://img.shields.io/badge/✨-Features-purple.svg)](docs/features.md) | [![Examples](https://img.shields.io/badge/🎨-Examples-cyan.svg)](docs/examples_EN.md)

</div>

## 🚀 Quick Start

### 📦 Installation
```bash
# Clone the repository
git clone https://github.com/yourusername/The_Linux_Command_Pro.git
cd The_Linux_Command_Pro

# Build the project
./build.sh

# Install to system (optional)
sudo make install
```

### 🎯 Quick Experience
```bash
# List files (with icons and colors)
pls

# Display file content (with syntax highlighting)
pcat main.c

# Search files
pfind . -name "*.c"

# Search text
pgrep "hello" file.txt

# Monitor processes
ptop

# Analyze disk usage
pdu
```

## 🎯 Command Overview

<div align="center">

| Command | Function | Features | Status |
|---------|----------|----------|--------|
| **pls** | File listing | 🎨 Colored output + file icons | ✅ Complete |
| **pcat** | File display | 🌈 Syntax highlighting + line numbers | ✅ Complete |
| **pfind** | File search | 🔍 Smart search + beautiful output | ✅ Complete |
| **pgrep** | Text search | 🎨 Highlight matches + statistics | ✅ Complete |
| **ptop** | Process monitoring | 📊 Real-time monitoring + colored display | 🚧 In Development |
| **pdu** | Disk usage | 📈 Graphical display + progress bar | 🚧 In Development |
| **pps** | Process list | 🌈 Colored processes + detailed info | 🚧 In Development |
| **pkill** | Process management | ⚡ Smart termination + interactive confirmation | 📋 Planned |
| **pfree** | Memory monitoring | 🧠 Memory visualization + real-time monitoring | 📋 Planned |
| **pwho** | User information | 👥 User login + status display | 📋 Planned |
| **puptime** | System status | ⏰ Uptime + load monitoring | 📋 Planned |
| **pawk** | Text processing | 🔧 Object-oriented + advanced features | 📋 Planned |
| **psed** | Stream editor | ✏️ Object-oriented + syntax highlighting | 📋 Planned |
| **pnetstat** | Network connections | 🌐 Connection visualization + statistics | 📋 Planned |
| **pmount** | Mount management | 💾 Mount point visualization + usage | 📋 Planned |
| **dsa** | Image viewer | 🖼️ ASCII art + terminal display | ✅ Complete |
| **pdate** | Date and time | 📅 Multiple formats + timezone support | ✅ Complete |
| **pecho** | Text output | 🎨 Escape characters + colored output | ✅ Complete |
| **pcal** | Calendar display | 📆 Beautiful calendar + highlight today | ✅ Complete |
| **pwhich** | Command finder | 🔍 Path lookup + detailed information | ✅ Complete |
| **pbc** | Basic calculator | 🧮 Math operations + interactive mode | ✅ Complete |
| **pseq** | Number sequence | 🔢 Sequence generation + formatting | ✅ Complete |
| **pman** | Manual pages | 📚 Manual pages + section support | ✅ Complete |
| **pwhereis** | File location | 🔍 File search + multiple type support | ✅ Complete |
| **pwhatis** | Command description | 📝 Command description + search function | ✅ Complete |
| **pshuf** | Random shuffle | 🎲 Random shuffle + selection function | ✅ Complete |
| **pyes** | Repeat output | 🔄 Repeat output + signal handling | ✅ Complete |
| **ptee** | Split output | 📤 Split output + multiple file support | ✅ Complete |
| **psplit** | File split | ✂️ Multiple split modes + progress display | ✅ Complete |
| **pjoin** | File merge | 🔗 Smart merge + multiple modes | ✅ Complete |
| **pdiff** | File comparison | 🔍 Colored diff + multiple formats | ✅ Complete |
| **pzip** | Compression | 📦 ZIP compression + progress display | ✅ Complete |
| **pcp** | File copy | 📋 Smart copy + progress display | ✅ Complete |
| **pmv** | File move | 🚚 Smart move + interactive confirmation | ✅ Complete |
| **ptar** | Archive tool | 📁 TAR archive + colored output | ✅ Complete |

</div>

> 📖 [View Detailed Usage Tutorial](docs/usage.md) | 🔧 [Installation Guide](docs/installation.md) | ✨ [Features](docs/features.md) | 🎨 [Command Examples](docs/examples_EN.md) | 📁 [Project Structure](docs/structure_EN.md)

## 🛠️ Tech Stack

<div align="center">

| Technology | Version | Description |
|------------|---------|-------------|
| **C Language** | C99 | Main development language |
| **Build System** | CMake 3.10+ | Cross-platform build |
| **Dependencies** | Standard C Library | No external dependencies |
| **Platform** | Linux | WSL2 supported |
| **Compiler** | GCC/Clang | C99 standard support |

</div>

## 🎨 Quick Preview

```bash
# File listing (with icons and colors)
$ pls

# File display (with syntax highlighting)
$ pcat -n main.c

# File search
$ pfind . -name "*.c"

# Text search
$ pgrep "hello" file.txt
```

> 🎨 [View More Command Examples](docs/examples_EN.md)

## 📚 Documentation

- 📖 [Usage Tutorial](docs/usage.md) - Detailed command usage and advanced tips
- 🔧 [Installation Guide](docs/installation.md) - System requirements and installation steps
- ✨ [Features](docs/features.md) - Project features and performance optimizations
- 🎨 [Command Examples](docs/examples_EN.md) - Real usage previews
- 📁 [Project Structure](docs/structure_EN.md) - Directory structure and file organization
- 🤝 [Contributing Guide](docs/contributing_EN.md) - How to contribute to the project
- 📊 [Development Status](docs/development_EN.md) - Development progress and version planning

## 🤝 Contributing

We welcome contributions of all kinds! Please see the [Contributing Guide](docs/contributing_EN.md) for details.

## 📊 Development Status

- ✅ **Completed**: 24 commands
- 🚧 **In Progress**: 3 commands
- 📋 **Planned**: 6+ commands

> 📊 [View Detailed Development Status](docs/development_EN.md)

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

