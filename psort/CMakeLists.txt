cmake_minimum_required(VERSION 3.10)
project(psort C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 创建psort可执行文件
add_executable(psort psort.c)

# 链接必要的库
target_link_libraries(psort PRIVATE common)

# 设置编译选项
target_compile_options(psort PRIVATE -Wall -Wextra -O2)

# 安装目标
install(TARGETS psort RUNTIME DESTINATION bin)
