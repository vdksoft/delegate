cmake_minimum_required(VERSION 3.8)

project(delegate-test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GTEST_SOURCE_DIR "" CACHE PATH "Path to Google Test sources")
set(GTEST_WORKING_DIR "${CMAKE_BINARY_DIR}/googletest")

if (NOT EXISTS "${GTEST_SOURCE_DIR}")

set(GTEST_SOURCE_DIR "${GTEST_WORKING_DIR}/src" CACHE PATH "Path to Google Test sources" FORCE)
configure_file("${CMAKE_CURRENT_LIST_DIR}/CMakeLists.in"
               "${GTEST_WORKING_DIR}/tmp/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
                WORKING_DIRECTORY "${GTEST_WORKING_DIR}/tmp")
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                WORKING_DIRECTORY "${GTEST_WORKING_DIR}/tmp")

endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("${GTEST_SOURCE_DIR}"
                 "${GTEST_WORKING_DIR}/build")
                 
add_executable(delegate-test "tests.cpp")
target_link_libraries(delegate-test gtest_main delegate)

add_test(delegate-test delegate-test)