# 安装指南

## 系统要求

### 最低要求
- Linux 内核 3.0+
- GCC 4.9+ 或 Clang 3.4+
- CMake 3.10+
- 标准C库

### 推荐配置
- Linux 内核 5.0+
- GCC 9.0+ 或 Clang 10.0+
- CMake 3.16+
- 2GB+ 内存

## 安装方法

### 方法一：从源码构建（推荐）

#### 1. 克隆项目
```bash
git clone https://github.com/yourusername/The_Linux_Command_Pro.git
cd The_Linux_Command_Pro
```

#### 2. 构建项目
```bash
# 使用构建脚本（推荐）
./build.sh

# 或手动构建
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### 3. 安装到系统
```bash
cd build
sudo make install
```

### 方法二：使用包管理器

#### Ubuntu/Debian
```bash
# 添加PPA（待发布）
sudo add-apt-repository ppa:yourusername/linux-command-pro
sudo apt update
sudo apt install linux-command-pro
```

#### CentOS/RHEL
```bash
# 添加YUM仓库（待发布）
sudo yum-config-manager --add-repo https://yourusername.github.io/The_Linux_Command_Pro/linux-command-pro.repo
sudo yum install linux-command-pro
```

#### Arch Linux
```bash
# 使用AUR（待发布）
yay -S linux-command-pro
```

### 方法三：预编译二进制文件

#### 下载预编译版本
```bash
# 下载最新版本
wget https://github.com/yourusername/The_Linux_Command_Pro/releases/latest/download/linux-command-pro-linux-x64.tar.gz

# 解压
tar -xzf linux-command-pro-linux-x64.tar.gz

# 安装
cd linux-command-pro-linux-x64
sudo ./install.sh
```

## 验证安装

### 检查安装
```bash
# 检查命令是否可用
pls --version
pcat --version
pfind --version
pgrep --version
```

### 测试功能
```bash
# 创建测试文件
echo "Hello World" > test.txt

# 测试pls
pls test.txt

# 测试pcat
pcat test.txt

# 测试pfind
pfind . -name "test.txt"

# 测试pgrep
pgrep "Hello" test.txt
```

## 卸载

### 从源码安装的卸载
```bash
cd build
sudo make uninstall
```

### 从包管理器安装的卸载
```bash
# Ubuntu/Debian
sudo apt remove linux-command-pro

# CentOS/RHEL
sudo yum remove linux-command-pro

# Arch Linux
sudo pacman -R linux-command-pro
```

## 配置

### 环境变量
```bash
# 添加到 ~/.bashrc 或 ~/.zshrc
export PATH="/usr/local/bin:$PATH"

# 启用颜色输出
export TERM=xterm-256color
```

### 别名设置
```bash
# 添加到 ~/.bashrc 或 ~/.zshrc
alias ls='pls'
alias cat='pcat'
alias find='pfind'
alias grep='pgrep'
```

## 故障排除

### 常见问题

#### 1. 编译错误
```bash
# 检查依赖
sudo apt install build-essential cmake

# 清理构建目录
rm -rf build
mkdir build && cd build
cmake ..
make
```

#### 2. 权限问题
```bash
# 检查安装权限
sudo chown -R $USER:$USER /usr/local/bin/pls*
```

#### 3. 库依赖问题
```bash
# 更新库缓存
sudo ldconfig

# 检查库依赖
ldd /usr/local/bin/pls
```

#### 4. 颜色显示问题
```bash
# 检查终端支持
echo $TERM

# 设置正确的终端类型
export TERM=xterm-256color
```

### 获取帮助
- 查看帮助信息：`pls --help`
- 查看版本信息：`pls --version`
- 提交Issue：https://github.com/yourusername/The_Linux_Command_Pro/issues
- 查看文档：https://github.com/yourusername/The_Linux_Command_Pro/wiki
