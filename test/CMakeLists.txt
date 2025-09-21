# 查找Google Test
find_package(GTest REQUIRED)

# 包含目录
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(../include)

# 创建测试可执行文件
add_executable(test_common test_common.cpp)
target_link_libraries(test_common common ${GTEST_LIBRARIES} pthread)

add_executable(test_pls test_pls.cpp)
target_link_libraries(test_pls common ${GTEST_LIBRARIES} pthread)

add_executable(test_pcat test_pcat.cpp)
target_link_libraries(test_pcat common ${GTEST_LIBRARIES} pthread)

add_executable(test_pgrep test_pgrep.cpp)
target_link_libraries(test_pgrep common ${GTEST_LIBRARIES} pthread)

# 运行测试
enable_testing()

add_test(NAME test_common COMMAND test_common)
add_test(NAME test_pls COMMAND test_pls)
add_test(NAME test_pcat COMMAND test_pcat)
add_test(NAME test_pgrep COMMAND test_pgrep)
