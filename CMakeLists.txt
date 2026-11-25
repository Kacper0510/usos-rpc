cmake_minimum_required(VERSION 3.24)  # Colored diagnostics, FIND_PACKAGE_ARGS
project(usos-rpc VERSION 1.1.0 HOMEPAGE_URL "github.com/Kacper0510/usos-rpc")
set(usos-rpc_COPYRIGHT "(c) 2024-2026 Kacper Wojciuch <kacper.b.wojciuch@gmail.com>")
message("[usos-rpc] Starting CMake configuration")

#[==============================[
  CMake setup
]==============================]#

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# For VS Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 'date' uses old CMake version
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

#[==============================[
  Main executable
]==============================]#

add_executable(usos-rpc)
target_include_directories(usos-rpc PRIVATE "src")
file(GLOB_RECURSE sources "src/*.cpp" "src/*.hpp")
target_sources(usos-rpc PRIVATE ${sources})

# No subfolders in bin on MSVC
set_property(TARGET usos-rpc PROPERTY RUNTIME_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/bin>)

# Do not define min and max macros on Windows, as they conflict with std::min and std::max
if(WIN32)
  target_compile_definitions(usos-rpc PRIVATE NOMINMAX)
endif()

#[==============================[
  Additional warnings
]==============================]#

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(usos-rpc INTERFACE
  "$<${gcc_like_cxx}:-Wall;-Wextra;-Wshadow;-Wunused;-pedantic>"
  "$<${msvc_cxx}:-W3>"
)

#[==============================[
  Link time optimizations
]==============================]#

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)
if(lto_supported)
  set_property(TARGET usos-rpc PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  set(CURL_LTO ON)
  message("[usos-rpc] Enabled link time optimization")
endif()

#[==============================[
  Dependencies
]==============================]#

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(usos-rpc_DEPENDENCIES "\n")


#[===[ git ]===]#
# Necessary for the build.
find_package(Git)
set(git_commit_string CACHE STRING "")
if(NOT GIT_EXECUTABLE AND git_commit_string STREQUAL "")
  message(FATAL_ERROR "[usos-rpc] 'git' not found - you need to install it!")
endif()


#[===[ libcurl ]===]#
# HTTP requests library.
set(curl_version curl-8_23_0)

set(BUILD_CURL_EXE OFF)
set(BUILD_STATIC_LIBS ON)
set(CURL_DISABLE_INSTALL ON)
set(ENABLE_UNICODE ON)
set(ENABLE_CURL_MANUAL OFF)
set(BUILD_LIBCURL_DOCS OFF)

# Enable default TLS backends
if(WIN32)
  set(CURL_USE_SCHANNEL ON)
else()
  set(CURL_USE_OPENSSL ON)
endif()

set(CURL_DISABLE_BASIC_AUTH ON)
set(CURL_DISABLE_BEARER_AUTH ON)
set(CURL_DISABLE_DIGEST_AUTH ON)
set(CURL_DISABLE_KERBEROS_AUTH ON)
set(CURL_DISABLE_NEGOTIATE_AUTH ON)
set(CURL_DISABLE_HTTP_AUTH ON)
set(CURL_DISABLE_AWS ON)
set(CURL_USE_LIBPSL OFF)
set(HTTP_ONLY ON)

FetchContent_Declare(
  CURL
  GIT_REPOSITORY "https://github.com/curl/curl.git"
  GIT_TAG ${curl_version}
  SYSTEM
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(CURL)
target_link_libraries(usos-rpc CURL::libcurl)

if(CURL_FOUND)
  message("[usos-rpc] Using system-provided 'libcurl'")
else()
  message("[usos-rpc] Completed fetching 'libcurl'")
endif()


#[===[ DiscordRPC ]===]#
# Official Discord Rich Presence library (deprecated, but still better than their SDK).
set(discord_rpc_version 963aa9f3e5ce81a4682c6ca3d136cddda614db33)

set(BUILD_EXAMPLES OFF)
set(USE_STATIC_CRT ON)

FetchContent_Declare(
  discord-rpc
  GIT_REPOSITORY "https://github.com/discord/discord-rpc.git"
  GIT_TAG ${discord_rpc_version}
  PATCH_COMMAND ${GIT_EXECUTABLE} apply ${PROJECT_SOURCE_DIR}/discord-rpc.patch
  UPDATE_DISCONNECTED ON
  SYSTEM
)
FetchContent_MakeAvailable(discord-rpc)
target_link_libraries(usos-rpc discord-rpc)
target_include_directories(usos-rpc PRIVATE "${discord-rpc_SOURCE_DIR}/include")

string(SUBSTRING ${discord_rpc_version} 0 7 discord_rpc_version)  # Shortened git hash
string(APPEND usos-rpc_DEPENDENCIES "\"DiscordRPC ${discord_rpc_version}\",\n")
message("[usos-rpc] Completed fetching 'DiscordRPC'")


#[===[ tomlplusplus ]===]#
# TOML config parser and serializer.
set(tomlplusplus_version v3.4.0)

FetchContent_Declare(
  tomlplusplus
  GIT_REPOSITORY "https://github.com/marzer/tomlplusplus.git"
  GIT_TAG ${tomlplusplus_version}
  SYSTEM
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(tomlplusplus)
target_link_libraries(usos-rpc tomlplusplus::tomlplusplus)

if(tomlplusplus_FOUND)
  message("[usos-rpc] Using system-provided 'tomlplusplus'")
else()
  message("[usos-rpc] Completed fetching 'tomlplusplus'")
endif()

string(APPEND usos-rpc_DEPENDENCIES "\"toml++ ${tomlplusplus_version}\",\n")


#[===[ battery-embed ]===]#
# Embedding files into the executable.
set(battery_embed_version v1.2.19)

set(B_PRODUCTION_MODE ON)

FetchContent_Declare(
  battery-embed
  GIT_REPOSITORY "https://github.com/batterycenter/embed.git"
  GIT_TAG ${battery_embed_version}
)
FetchContent_MakeAvailable(battery-embed)
string(APPEND usos-rpc_DEPENDENCIES "\"battery-embed ${battery_embed_version}\",\n")
message("[usos-rpc] Completed fetching 'battery-embed'")

#[==============================[
  Embedded files
]==============================]#

# Fixes include path for clang-tidy
target_include_directories(usos-rpc PRIVATE "${EMBED_BINARY_DIR}/autogen/usos-rpc/include/battery")

# Embed files
file(GLOB_RECURSE resources RELATIVE "${PROJECT_SOURCE_DIR}/resources" "resources/*")
foreach(resource ${resources})
  b_embed(usos-rpc "resources/${resource}")
endforeach()
string(REPLACE ";" " " resources_str "${resources}")
message("[usos-rpc] Embedded following files: ${resources_str}")

# Include info about embedded files in build info header
set(usos-rpc_EMBEDDED_FILES "\n")
foreach(resource ${resources})
  if(resource MATCHES ".ansi$" OR resource MATCHES ".service$")  # Skip automatic unpacking for certain files
    continue()
  endif()
  if(WIN32)
    # Path separator fix
    string(REPLACE "/" "\\\\" resource_name ${resource})
  else()
    set(resource_name ${resource})
  endif()
  string(APPEND usos-rpc_EMBEDDED_FILES "{\"${resource_name}\", b::embed<\"resources/${resource}\">().data()},\n")
endforeach()

#[==============================[
  Build information header
]==============================]#

if(git_commit_string STREQUAL "")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --dirty --exclude '*'
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE git_commit_string
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
endif()

if(git_commit_string STREQUAL "")
  set(usos-rpc_FULL_VERSION "${CMAKE_PROJECT_VERSION}")
else()
  set(usos-rpc_FULL_VERSION "${CMAKE_PROJECT_VERSION}+${git_commit_string}")
endif()

target_include_directories(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}/generated")
configure_file("src/build_info.hpp.in" "generated/build_info.hpp")
target_sources(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}/generated/build_info.hpp")
message("[usos-rpc] Exported '${usos-rpc_FULL_VERSION}' to build_info.hpp")
