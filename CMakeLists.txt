cmake_minimum_required(VERSION 3.22)
project(dotembed)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(unofficial-nethost CONFIG REQUIRED)

add_library(dotembed SHARED src/dotembed.cpp src/dotembed.h)
target_link_libraries(dotembed PRIVATE unofficial::nethost::nethost)

add_executable(dotest src/main.cpp)
target_link_libraries(dotest dotembed)

