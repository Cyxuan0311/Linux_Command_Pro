# pget 命令
add_executable(pget pget.c)
target_link_libraries(pget common)

# 尝试查找 libcurl
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(CURL libcurl)
    if(CURL_FOUND)
        message(STATUS "Found libcurl: ${CURL_VERSION}")
        target_include_directories(pget PRIVATE ${CURL_INCLUDE_DIRS})
        target_link_libraries(pget ${CURL_LIBRARIES})
        target_compile_definitions(pget PRIVATE HAVE_LIBCURL)
    else()
        message(WARNING "libcurl not found. pget will have limited functionality.")
        message(STATUS "To enable full functionality, install libcurl:")
        message(STATUS "  Ubuntu/Debian: sudo apt-get install libcurl4-openssl-dev")
        message(STATUS "  CentOS/RHEL:   sudo yum install libcurl-devel")
        message(STATUS "  Arch Linux:    sudo pacman -S curl")
    endif()
else()
    message(WARNING "PkgConfig not found. Cannot check for libcurl.")
    message(STATUS "pget will have limited functionality without libcurl.")
endif()


