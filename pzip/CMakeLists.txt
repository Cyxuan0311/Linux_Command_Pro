# pzip 命令
add_executable(pzip pzip.c)
target_link_libraries(pzip common)
# 需要链接zlib库
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZLIB REQUIRED zlib)
target_link_libraries(pzip ${ZLIB_LIBRARIES})
target_include_directories(pzip PRIVATE ${ZLIB_INCLUDE_DIRS})
