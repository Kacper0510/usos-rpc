cmake_minimum_required(VERSION 3.22)
project(usos-rpc VERSION 1.0.0)

# CMake setup
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../bin")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_SKIP_INSTALL_RULES ON)

# Options
option(USOS_RPC_STD_ONLY "Disables system specific headers")

# Main executable
add_executable(usos-rpc)
target_include_directories(usos-rpc PRIVATE "src")
file(GLOB sources "src/*.cpp" "src/*.hpp")
target_sources(usos-rpc PRIVATE ${sources})
if(USOS_RPC_STD_ONLY)
  target_compile_definitions(usos-rpc PRIVATE USOS_RPC_STD_ONLY)
  message("[usos-rpc] Disabled system specific library calls")
endif()

# Version information header
target_include_directories(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}")
configure_file("src/version_info.hpp.in" "version_info.hpp")
target_sources(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}/version_info.hpp")
message("[usos-rpc] Exported version_info.hpp")

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
  message("[usos-rpc] Enabled link time optimization")
endif()

# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

## fmt
### Text formatting library, implementation of C++23 <format> header.
set(fmt_version 10.2.1)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
  GIT_TAG "${fmt_version}"
)
FetchContent_MakeAvailable(fmt)
target_link_libraries(usos-rpc fmt::fmt)
message("[usos-rpc] Completed fetching 'fmt'")

## libcurl
### HTTP requests library.
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
  OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(curl)
set(CURL_LIBRARIES libcurl)
target_link_libraries(usos-rpc libcurl)
message("[usos-rpc] Completed fetching 'libcurl'")

## date
### Date and time library, implementation of C++20 <chrono> header.
set(date_version 3.0.1)

# FIXME: find a better solution so that date does not use curl and 7zip...
# when reverting, delete all other unnecessary statements
if(WIN32)
  target_compile_definitions(usos-rpc PRIVATE NOMINMAX)
else()
  set(USE_SYSTEM_TZ_DB ON)
endif()

set(BUILD_TZ_LIB ON)
FetchContent_Declare(
  date_src
  GIT_REPOSITORY "https://github.com/HowardHinnant/date.git"
  GIT_TAG "v${date_version}"
)
FetchContent_MakeAvailable(date_src)
target_link_libraries(usos-rpc date::date)
target_link_libraries(usos-rpc date::date-tz)
message("[usos-rpc] Completed fetching 'date'")
