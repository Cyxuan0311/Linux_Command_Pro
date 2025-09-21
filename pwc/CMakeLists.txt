cmake_minimum_required(VERSION 3.10)
project(pwc C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 创建pwc可执行文件
add_executable(pwc pwc.c)

# 链接必要的库
target_link_libraries(pwc PRIVATE common)

# 设置编译选项
target_compile_options(pwc PRIVATE -Wall -Wextra -O2)

# 安装目标
install(TARGETS pwc RUNTIME DESTINATION bin)
