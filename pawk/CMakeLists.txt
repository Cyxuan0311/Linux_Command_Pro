cmake_minimum_required(VERSION 3.10)
project(pawk CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(PAWK_SOURCES
    src/pawk_new.cpp
    src/Utils.cpp
    src/Field.cpp
    src/Record.cpp
    src/BuiltinFunctions.cpp
    src/Variables.cpp
    src/Pattern.cpp
    src/Action.cpp
    src/CommandLineParser.cpp
    src/AwkEngine.cpp
)

# 创建可执行文件
add_executable(pawk ${PAWK_SOURCES})
target_link_libraries(pawk common)

# 设置编译选项
target_compile_options(pawk PRIVATE -Wall -Wextra)

# 创建旧版本的可执行文件（向后兼容）
add_executable(pawk_old pawk.cpp)
target_link_libraries(pawk_old common)
target_compile_options(pawk_old PRIVATE -Wall -Wextra)