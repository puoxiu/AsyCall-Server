cmake_minimum_required(VERSION 3.10)

# 项目名称
project(x-http-1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}/buffer
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/epoll
    ${PROJECT_SOURCE_DIR}/http
    ${PROJECT_SOURCE_DIR}/server
    ${PROJECT_SOURCE_DIR}/utils
)

# 源文件
file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/core/*.cpp"
    "${PROJECT_SOURCE_DIR}/epoll/*.cpp"
    "${PROJECT_SOURCE_DIR}/http/*.cpp"
    "${PROJECT_SOURCE_DIR}/server/*.cpp"
    "${PROJECT_SOURCE_DIR}/utils/*.cpp"
    "${PROJECT_SOURCE_DIR}/main.cpp"
)

# 添加可执行文件
add_executable(server ${SOURCES})

# 手动指定 fmt 库路径
find_path(FMT_INCLUDE_DIR fmt/core.h PATHS /usr/local/include)
find_library(FMT_LIBRARY fmt PATHS /usr/local/lib)

if(NOT FMT_INCLUDE_DIR OR NOT FMT_LIBRARY)
    message(FATAL_ERROR "fmt library not found")
endif()

# 链接 fmt 库
target_include_directories(server PRIVATE ${FMT_INCLUDE_DIR})
target_link_libraries(server PRIVATE ${FMT_LIBRARY})

# 编译选项（可选）
# add_compile_options(-Wall -Wextra -pedantic)

# 安装规则（可选）
# install(TARGETS x-http-1.0 DESTINATION bin)
# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)