cmake_minimum_required(VERSION 3.8)

project(delegate CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(delegate INTERFACE)
target_sources(delegate INTERFACE ${CMAKE_SOURCE_DIR}/src/delegate.h)
target_include_directories(delegate INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(delegate INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    $<$<CXX_COMPILER_ID:Clang>:-Wall>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>)

add_executable(delegate-demo ${CMAKE_SOURCE_DIR}/demo/main.cpp)
target_link_libraries(delegate-demo delegate)

add_subdirectory(tests)