cmake_minimum_required(VERSION 3.10)
project(pmkdir C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 创建pmkdir可执行文件
add_executable(pmkdir pmkdir.c)

# 链接必要的库
target_link_libraries(pmkdir PRIVATE common)

# 设置编译选项
target_compile_options(pmkdir PRIVATE -Wall -Wextra -O2)

# 安装目标
install(TARGETS pmkdir RUNTIME DESTINATION bin)
