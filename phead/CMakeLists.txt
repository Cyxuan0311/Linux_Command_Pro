cmake_minimum_required(VERSION 3.10)
project(phead C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 查找getopt库（如果需要）
find_package(PkgConfig)

# 创建phead可执行文件
add_executable(phead phead.c)

# 链接必要的库
target_link_libraries(phead PRIVATE common)

# 设置编译选项
target_compile_options(phead PRIVATE -Wall -Wextra -O2)

# 安装目标
install(TARGETS phead RUNTIME DESTINATION bin)
