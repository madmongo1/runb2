cmake_minimum_required(VERSION 3.15)

include(cmake/HunterGate.cmake)
include(cmake/Boostify.cmake)

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.240.tar.gz"
        SHA1 "ca19f3769e6c80cfdd19d8b12ba5102c27b074e0"
)

project(RunB2)

hunter_add_package(range-v3)

find_package(Threads)
find_package(range-v3 CONFIG REQUIRED)

file(GLOB_RECURSE src_files CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp")
add_executable(runb2 ${src_files})
target_include_directories(runb2 PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src)
boostify(runb2 system filesystem program_options)
target_link_libraries(runb2 PRIVATE range-v3 Threads::Threads)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(runb2 PRIVATE -Werror -Wall -Wextra -pedantic)
endif ()

install(TARGETS runb2)
