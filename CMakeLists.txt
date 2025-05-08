cmake_minimum_required(VERSION 3.30)
project(DexDump)

set(CMAKE_CXX_STANDARD 20)

# 设置输出目录（相对于构建目录，如 cmake-build-debug/out）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)  # 可执行文件（.exe）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)  # 静态库（.lib）
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)  # 动态库（.dll）

# 可选：对象文件（.obj）输出目录（需 CMake 3.13+）
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)


add_executable(DexDump src/Test.cpp)
target_include_directories(DexDump PRIVATE ${PROJECT_SOURCE_DIR}/include)
