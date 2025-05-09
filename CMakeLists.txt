cmake_minimum_required(VERSION 3.22.1)
project(DexDump)

set(CMAKE_CXX_STANDARD 20)

# 设置输出目录（相对于构建目录，如 cmake-build-debug/out）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)  # 可执行文件（.exe）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)  # 静态库（.lib）
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)  # 动态库（.dll）

# 可选：对象文件（.obj）输出目录（需 CMake 3.13+）
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)


add_executable(DexDump
#        src/Test.cpp
        test/dexdump_main.cpp
        include/core/util.cpp
        include/core/util.h
        include/log/log.h
        include/log/log.cpp
        include/core/DexContext.h
        include/core/DexContext.cpp
        include/formatter/StringPrint.cpp
        include/formatter/StringPrint.h
        include/formatter/ProtoPrint.cpp
        include/formatter/ProtoPrint.h
        include/parser/FieldParser.cpp
        include/parser/FieldParser.h
        include/formatter/FieldPrint.cpp
        include/formatter/FieldPrint.h
        include/formatter/MethodPrint.cpp
        include/formatter/MethodPrint.h
        include/parser/MethodParser.cpp
        include/parser/MethodParser.h
        include/core/DexDump.cpp
        include/formatter/ClassPrint.cpp
        include/formatter/ClassPrint.h
        include/formatter/FormatUtil.cpp
        include/formatter/FormatUtil.h
        include/parser/ClassDefsParser.cpp
        include/parser/ClassDefsParser.h
        include/core/DexDump.h
        include/parser/BaseParser.cpp
        include/parser/BaseParser.h
        include/parser/StringPoolParser.cpp
        include/parser/StringPoolParser.h
        include/parser/HeaderParser.cpp
        include/parser/HeaderParser.h
        include/formatter/BasePrint.cpp
        include/formatter/BasePrint.h
        include/formatter/HeaderPrint.cpp
        include/formatter/HeaderPrint.h
        include/parser/TypeParser.cpp
        include/parser/TypeParser.h
        include/formatter/TypePrint.cpp
        include/formatter/TypePrint.h
        src/main.cpp
        include/parser/ProtoParser.cpp
        include/parser/ProtoParser.h
        include/parser/CodeParser.cpp
        include/parser/CodeParser.h
        include/formatter/CodePrint.cpp
        include/formatter/CodePrint.h
        include/formatter/DebugInfoPrint.cpp
        include/formatter/DebugInfoPrint.h)
target_include_directories(DexDump PRIVATE ${PROJECT_SOURCE_DIR}/include)
