cmake_minimum_required(VERSION 3.27)
project(usos-rpc VERSION 1.0.0)

# CMake setup
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Main executable
add_executable(usos-rpc)
target_include_directories(usos-rpc PRIVATE "src")
file(GLOB sources "src/*.cpp" "src/*.hpp")
target_sources(usos-rpc PRIVATE sources)

# Version information header
target_include_directories(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}")
configure_file("src/version_info.hpp.in" "version_info.hpp")
target_sources(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}/version_info.hpp")
