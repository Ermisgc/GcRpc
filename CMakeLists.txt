# CMakeList.txt: learn_cmake 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.5)

SET(CMAKE_C_COMPILER /usr/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)

# 可以指定更多的信息，比如加一个版本号
project ("GcRpc")

# C++编译器设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

# using gdb
set(CMAKE_BUILD_TYPE "Debug")

# .exe's output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# .lib's output path
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# header files' searching path
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# link libs
link_directories(${PROJECT_SOURCE_DIR}/lib)

# about rpc 
add_subdirectory(src)

# about example
add_subdirectory(example)
