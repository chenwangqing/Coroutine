# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.5)

# 项目信息
project(LibCoroutine VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_SYSTEM_NAME Linux)

SET(CMAKE_C_COMPILER gcc)
SET(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-exceptions ") # 设置c++的编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 ") # 设置c的编译选项

# -U_FORTIFY_SOURCE     禁用运输时栈检查
# -Wno-unused-result    忽略返回值
# -fPIC                 连接生成的动态库
# -fvisibility=hidden   默认不公开函数
# -fno-exceptions       不使用异常
# -fno-rtti             不需要RTTI
add_definitions(" -Wall -lc -lgcc -D_GNU_SOURCE -Wno-unused-result -Wformat=0 -U_FORTIFY_SOURCE -fPIC -fvisibility=hidden ")

if(DEBUG)
    message("GDB Debug")

    # 允许GDB 调试
    set(CMAKE_BUILD_TYPE "Debug")

    # 添加调试选项
    add_definitions("-g -DDEBUG -ggdb")
else()
    message("Release")
    set(CMAKE_BUILD_TYPE "Release")
    add_definitions("-Os")
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
set(INCLUDE_DIR ${INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../src")

# 添加 pthread 多线程库
set(EXTRA_LIBS ${EXTRA_LIBS} libpthread.so)

# 添加 动态库的支持
set(EXTRA_LIBS ${EXTRA_LIBS} libdl.so)

# 添加 Posix的实时扩展库
set(EXTRA_LIBS ${EXTRA_LIBS} librt.so)

set(CODE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Coroutine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/NodeLink.c)


# ------------------------------------- 编译 --------------------------------------
message("CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
message("INCLUDE_DIR=${INCLUDE_DIR}")
message("EXTRA_LIBS=${EXTRA_LIBS}")

# 头文件
include_directories(${INCLUDE_DIR})

# 输出执行程序
add_executable(${PROJECT_NAME} ${CODE_SRCS})
target_link_libraries(${PROJECT_NAME} -static-libgcc -static-libstdc++) # 静态链接到glibc
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS})

# 输出MAP文件
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Map=${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.map")

