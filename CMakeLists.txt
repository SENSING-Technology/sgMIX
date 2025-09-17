cmake_minimum_required(VERSION 3.10)
project(sgmix_demo)

set(CMAKE_CXX_STANDARD 17)

# 包含头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 设置动态库路径（可选，但方便运行时找库）
link_directories(/usr/lib)

# 添加 sample 目录中的 main 文件
add_executable(demo sample/main.cpp)

# 链接动态库
target_link_libraries(demo sgmix z)
