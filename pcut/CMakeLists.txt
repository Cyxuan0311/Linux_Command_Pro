cmake_minimum_required(VERSION 3.10)
project(pcut C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 创建pcut可执行文件
add_executable(pcut pcut.c)

# 链接必要的库
target_link_libraries(pcut PRIVATE common)

# 设置编译选项
target_compile_options(pcut PRIVATE -Wall -Wextra -O2)

# 安装目标
install(TARGETS pcut RUNTIME DESTINATION bin)
