cmake_minimum_required(VERSION 3.10)
project(ptouch C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 创建ptouch可执行文件
add_executable(ptouch ptouch.c)

# 链接必要的库
target_link_libraries(ptouch PRIVATE common)

# 设置编译选项
target_compile_options(ptouch PRIVATE -Wall -Wextra -O2)

# 安装目标
install(TARGETS ptouch RUNTIME DESTINATION bin)
