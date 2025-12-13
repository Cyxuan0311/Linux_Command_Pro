cmake_minimum_required(VERSION 3.10)
project(LinuxCommandPro)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# 包含目录
include_directories(include)

# 创建common库
add_library(common src/common.c)

# 添加子目录
add_subdirectory(pls)
add_subdirectory(pcat)
add_subdirectory(pfind)
add_subdirectory(pgrep)
add_subdirectory(ptop)
add_subdirectory(pdu)
add_subdirectory(pps)
add_subdirectory(pdf)
add_subdirectory(pkill)
add_subdirectory(pfree)
add_subdirectory(pwho)
add_subdirectory(puptime)
add_subdirectory(pw)
add_subdirectory(pawk)
add_subdirectory(psed)
add_subdirectory(pnetstat)
add_subdirectory(pmount)
add_subdirectory(pvim)
add_subdirectory(phead)
add_subdirectory(ptail)
add_subdirectory(pcut)
add_subdirectory(psort)
add_subdirectory(puniq)
add_subdirectory(pwc)
add_subdirectory(ptouch)
add_subdirectory(pmkdir)
add_subdirectory(prm)
add_subdirectory(dsa)
add_subdirectory(pdate)
add_subdirectory(pecho)
add_subdirectory(pcal)
add_subdirectory(pwhich)
add_subdirectory(pbc)
add_subdirectory(pseq)
add_subdirectory(pman)
add_subdirectory(pwhereis)
add_subdirectory(pwhatis)
add_subdirectory(pshuf)
add_subdirectory(pyes)
add_subdirectory(ptee)
add_subdirectory(psplit)
add_subdirectory(pjoin)
add_subdirectory(pdiff)
add_subdirectory(pzip)
add_subdirectory(pcp)
add_subdirectory(pmv)
add_subdirectory(ptar)
add_subdirectory(pcpu)
add_subdirectory(pget)

add_subdirectory(phistory)

# 安装目标
install(TARGETS pls pcat pfind pgrep ptop pdu pps pdf pkill pfree pwho puptime pw pawk psed pnetstat pmount pvim phead ptail pcut psort puniq pwc ptouch pmkdir prm dsa pdate pecho pcal pwhich pbc pseq pman pwhereis pwhatis pshuf pyes ptee psplit pjoin pdiff pzip pcp pmv ptar pcpu pget phistory
        RUNTIME DESTINATION bin)

# 测试支持
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
