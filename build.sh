#!/bin/bash

# Linux Command Pro 构建脚本

echo "🚀 开始构建 Linux Command Pro..."

# 创建构建目录
mkdir -p build
cd build

# 运行CMake配置
echo "📋 配置项目..."
cmake ..

# 编译项目
echo "🔨 编译项目..."
make -j$(nproc)

# 检查编译结果
if [ $? -eq 0 ]; then
    echo "✅ 编译成功！"
    echo ""
    echo "📦 可执行文件位置："
    echo "  - pls  (优化版 ls):  $(pwd)/pls/pls"
    echo "  - pcat (优化版 cat): $(pwd)/pcat/pcat"
    echo "  - pfind(优化版 find):$(pwd)/pfind/pfind"
    echo "  - pgrep(优化版 grep):$(pwd)/pgrep/pgrep"
    echo " ... "
    echo ""
    echo "💡 使用方法："
    echo "  ./pls/pls --help"
    echo "  ./pcat/pcat --help"
    echo "  ./pfind/pfind --help"
    echo "  ./pgrep/pgrep --help"
    echo " ... "
    echo ""
    echo "🔧 安装到系统："
    echo "  sudo make install"
else
    echo "❌ 编译失败！"
    exit 1
fi
