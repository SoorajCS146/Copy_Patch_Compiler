cmake_minimum_required(VERSION 3.10)
project(stencil_compiler C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)

file(GLOB SRC_FILES "src/*.cpp")

add_executable(stencilc ${SRC_FILES})
