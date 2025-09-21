cmake_minimum_required(VERSION 3.10)
project(pvim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(PVIM_SOURCES
    src/pvim.cpp
    src/Editor.cpp
    src/Buffer.cpp
    src/Window.cpp
    src/CommandMode.cpp
    src/InsertMode.cpp
    src/VisualMode.cpp
    src/FileManager.cpp
    src/SyntaxHighlighter.cpp
    src/KeyHandler.cpp
    src/StatusBar.cpp
    src/Config.cpp
    modules/TextProcessor.cpp
    modules/UndoManager.cpp
    modules/SearchEngine.cpp
    modules/PluginManager.cpp
)

# 创建可执行文件
add_executable(pvim ${PVIM_SOURCES})
target_link_libraries(pvim common)

# 设置编译选项
target_compile_options(pvim PRIVATE -Wall -Wextra)

# 如果需要ncurses支持
find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)
target_link_libraries(pvim ${NCURSES_LIBRARIES})
target_include_directories(pvim PRIVATE ${NCURSES_INCLUDE_DIRS})
