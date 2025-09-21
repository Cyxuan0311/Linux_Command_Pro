cmake_minimum_required(VERSION 3.10)
project(ptail C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 创建ptail可执行文件
add_executable(ptail ptail.c)

# 链接必要的库
target_link_libraries(ptail PRIVATE common)

# 设置编译选项
target_compile_options(ptail PRIVATE -Wall -Wextra -O2)

# 安装目标
install(TARGETS ptail RUNTIME DESTINATION bin)
