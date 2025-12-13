# 📁 Project Structure

This document details the directory structure and file organization of the Linux Command Pro project.

## Directory Tree

```
The_Linux_Command_Pro/
├── 📄 README.md              # Project description (Chinese)
├── 📄 README_EN.md           # Project description (English)
├── 🔧 build.sh               # Build script
├── 📋 CMakeLists.txt         # Main CMake configuration
├── 📁 docs/                  # Documentation directory
│   ├── 📖 usage.md           # Usage tutorial
│   ├── 🔧 installation.md    # Installation guide
│   ├── ✨ features.md        # Features
│   ├── 🎨 examples.md        # Command preview examples
│   ├── 🤝 contributing.md    # Contributing guide
│   ├── 📊 development.md     # Development status
│   └── 📁 structure.md       # Project structure (this file)
├── 📁 include/               # Header files
│   └── 📄 common.h           # Common header file
├── 📁 src/                   # Source code
│   └── 📄 common.c           # Common implementation
├── 📁 pls/                   # pls command
│   ├── 📋 CMakeLists.txt
│   └── 📄 pls.c
├── 📁 pcat/                  # pcat command
│   ├── 📋 CMakeLists.txt
│   └── 📄 pcat.c
├── 📁 pfind/                 # pfind command
│   ├── 📋 CMakeLists.txt
│   └── 📄 pfind.c
├── 📁 pgrep/                 # pgrep command
│   ├── 📋 CMakeLists.txt
│   └── 📄 pgrep.c
├── 📁 ptop/                  # ptop command
│   ├── 📋 CMakeLists.txt
│   └── 📄 ptop.c
├── 📁 pdu/                   # pdu command
│   ├── 📋 CMakeLists.txt
│   └── 📄 pdu.c
└── 📁 pps/                   # pps command
    ├── 📋 CMakeLists.txt
    └── 📄 pps.c
```

## Directory Description

### Root Directory
- **README.md** / **README_EN.md**: Main project documentation with quick start and overview
- **build.sh**: Automated build script
- **CMakeLists.txt**: Main CMake configuration file

### docs/
Documentation directory containing all project documentation:
- **usage.md**: Detailed usage tutorial and command descriptions
- **installation.md**: Installation guide and system requirements
- **features.md**: Feature introduction
- **examples.md**: Command usage examples and previews
- **contributing.md**: Contributing guide and development standards
- **development.md**: Development status and version planning
- **structure.md**: Project structure description (this file)

### include/
Common header files directory:
- **common.h**: Contains constants and function declarations shared by all commands

### src/
Common source code directory:
- **common.c**: Implementation of common functions, such as color output, file type identification, etc.

### Command Directories
Each command has its own directory containing:
- **CMakeLists.txt**: CMake configuration for that command
- **command_name.c**: Source code implementation of the command

## File Naming Conventions

- Command directories: lowercase, e.g., `pls/`, `pcat/`
- Source files: lowercase, e.g., `pls.c`, `common.c`
- Header files: lowercase, e.g., `common.h`
- Documentation files: lowercase with underscores, e.g., `usage.md`, `installation.md`

## Build Output

Built files are typically located in the `build/` directory:
```
build/
├── pls
├── pcat
├── pfind
└── ...
```

## Extending the Project

When adding a new command:
1. Create a new command directory in the root directory
2. Add `CMakeLists.txt` and source file
3. Add subdirectory to main `CMakeLists.txt`
4. Update relevant documentation

> 📖 For more information, see [Contributing Guide](contributing.md)

