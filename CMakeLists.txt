cmake_minimum_required(VERSION 3.22)
project(usos-rpc VERSION 1.0.0)

# CMake setup
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../bin")
set(BUILD_SHARED_LIBS OFF)

# Options
option(USOS_RPC_STD_ONLY "Disables system specific headers")

# Main executable
add_executable(usos-rpc)
target_include_directories(usos-rpc PRIVATE "src")
file(GLOB sources "src/*.cpp" "src/*.hpp")
target_sources(usos-rpc PRIVATE ${sources})
if(USOS_RPC_STD_ONLY)
  target_compile_definitions(usos-rpc PRIVATE USOS_RPC_STD_ONLY)
endif()

# Version information header
target_include_directories(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}")
configure_file("src/version_info.hpp.in" "version_info.hpp")
target_sources(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}/version_info.hpp")

# Additional warnings
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(usos-rpc INTERFACE
  "$<${gcc_like_cxx}:-Wall;-Wextra;-Wshadow;-Wunused;-pedantic>"
  "$<${msvc_cxx}:-W3>"
)

# Link time optimizations
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)
if(lto_supported)
  set_property(TARGET usos-rpc PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  set(CURL_LTO ON)
endif()

# Dependencies
include(FetchContent)

## fmt
set(fmt_version 10.2.1)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
  GIT_TAG "${fmt_version}"
)
FetchContent_MakeAvailable(fmt)
target_link_libraries(usos-rpc fmt::fmt)

## libcurl
set(curl_version 8_7_1)
set(BUILD_CURL_EXE OFF)
set(BUILD_STATIC_LIBS ON)
set(CURL_DISABLE_INSTALL ON)
set(ENABLE_UNICODE ON)
set(ENABLE_CURL_MANUAL OFF)
set(BUILD_LIBCURL_DOCS OFF)
### Enable default TLS backends
if(WIN32)
  set(CURL_USE_SCHANNEL ON)
else()
  set(CURL_USE_OPENSSL ON)
endif()
FetchContent_Declare(
  curl
  GIT_REPOSITORY "https://github.com/curl/curl.git"
  GIT_TAG "curl-${curl_version}"
)
FetchContent_MakeAvailable(curl)
target_link_libraries(usos-rpc libcurl)
