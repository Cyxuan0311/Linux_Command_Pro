cmake_minimum_required(VERSION 3.10)
project(puniq C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 创建puniq可执行文件
add_executable(puniq puniq.c)

# 链接必要的库
target_link_libraries(puniq PRIVATE common)

# 设置编译选项
target_compile_options(puniq PRIVATE -Wall -Wextra -O2)

# 安装目标
install(TARGETS puniq RUNTIME DESTINATION bin)
