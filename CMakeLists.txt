#cmake最低版本
cmake_minimum_required(VERSION 3.5)

# 设置cpu类型
set(TARGET_CPU "X86")
# set(TARGET_CPU "8100")
# set(TARGET_CPU "jcore")
# set(TARGET_CPU "aarm64")

#设置调试开关
option(DEBUG_ENABLED "Enable debug" ON)
if (DEBUG_ENABLED)
    set(GNU_FLAGS "-O0 -g") 
else()
    set(GNU_FLAGS "-O2 -s ")
endif()

if(TARGET_CPU STREQUAL "X86") 
    set(CMAKE_CXX_FLAGS "${GNU_FLAGS} -fpermissive")
    #set(CMAKE_C_FLAGS "${GNU_FLAGS}  ")
endif()

#AARM64 交叉编译选项
if(TARGET_CPU STREQUAL "aarm64") 
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarm64)
    set(TOOLCHAIN_DIR "/opt/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu")
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-g++)
    set(CMAKE_C_COMPILER   ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-gcc)

    set(CMAKE_CXX_FLAGS "${GNU_FLAGS} ")
    set(CMAKE_C_FLAGS "${GNU_FLAGS}  ")
    set(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_DIR}
        ${TOOLCHAIN_DIR}/aarch64-linux-gnu/include
        ${TOOLCHAIN_DIR}/aarch64-linux-gnu/lib )

    message("CPU=aarm64 CPU_TYPE=${CMAKE_SYSTEM_PROCESSOR}")
endif() 

#moxa 8100 交叉编译选项
if(TARGET_CPU STREQUAL "8100") 
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    set(TOOLCHAIN_DIR "/opt/armbuildtool")
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)
    set(CMAKE_C_COMPILER   ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)

    set(CMAKE_CXX_FLAGS "${GNU_FLAGS} ")
    set(CMAKE_C_FLAGS "${GNU_FLAGS}  ")
    set(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_DIR}
        ${TOOLCHAIN_DIR}/arm-linux-gnueabihf/include
        ${TOOLCHAIN_DIR}/arm-linux-gnueabihf/lib )

    message("CPU=8100 CPU_TYPE=${CMAKE_SYSTEM_PROCESSOR}")
endif() 

#jcore 交叉编译选项
if(TARGET_CPU STREQUAL "jcore")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    set(TOOLCHAIN_DIR "/opt/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf")
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)
    set(CMAKE_C_COMPILER   ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)

    set(CMAKE_CXX_FLAGS "${GNU_FLAGS} ")
    set(CMAKE_C_FLAGS "${GNU_FLAGS}  ")
    set(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_DIR}
        ${TOOLCHAIN_DIR}/arm-linux-gnueabihf/include
        ${TOOLCHAIN_DIR}/arm-linux-gnueabihf/lib )

    message("CPU=jcore CPU_TYPE=${CMAKE_SYSTEM_PROCESSOR}")
endif() 


set(JGATE_MAJOR_VERSION 1)
set(JGATE_MINOR_VERSION 1)
set(JGATE_PATCH_VERSION 0)
set(JGATE_FULL_VERSION ${JGATE_MAJOR_VERSION}.${JGATE_MINOR_VERSION}.${JGATE_PATCH_VERSION})

project(jincubator VERSION ${JGATE_FULL_VERSION})

#set(CMAKE_CXX_STANDARD 11)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(zlog)




