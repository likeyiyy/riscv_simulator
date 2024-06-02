cmake_minimum_required(VERSION 3.19)

# 项目信息
project(RISCSimulator VERSION 1.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找源文件
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

# 设置编译选项以启用调试信息
set(CMAKE_C_FLAGS_DEBUG "-g")

# 生成可执行文件
add_executable(riscv_simulator ${SOURCES})

# 设置编译选项
target_compile_options(riscv_simulator PRIVATE -Wall -Wextra -Wpedantic)
