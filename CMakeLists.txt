cmake_minimum_required(VERSION 3.22)
project(usos-rpc VERSION 0.1)

#[==============================[
  CMake setup
]==============================]#

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)
set(BUILD_SHARED_LIBS OFF)

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

#[==============================[
  Version information header
]==============================]#

find_package(Git)
if(GIT_EXECUTABLE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --dirty --exclude '*'
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE git_commit_string
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
if(git_commit_string STREQUAL "")
  set(usos-rpc_FULL_VERSION "${CMAKE_PROJECT_VERSION}")
else()
  set(usos-rpc_FULL_VERSION "${CMAKE_PROJECT_VERSION}+${git_commit_string}")
endif()

target_include_directories(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}")
configure_file("src/version_info.hpp.in" "version_info.hpp")
target_sources(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}/version_info.hpp")
message("[usos-rpc] Exported '${usos-rpc_FULL_VERSION}' to version_info.hpp")

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


#[===[ fmt ]===]#
# Text formatting library, implementation of C++23 <format> header.
set(fmt_version 10.2.1)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
  GIT_TAG "${fmt_version}"
)
FetchContent_MakeAvailable(fmt)
target_link_libraries(usos-rpc fmt::fmt)
message("[usos-rpc] Completed fetching 'fmt'")


#[===[ libcurl ]===]#
# HTTP requests library.
set(curl_version 8_7_1)

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

FetchContent_Declare(
  curl
  GIT_REPOSITORY "https://github.com/curl/curl.git"
  GIT_TAG "curl-${curl_version}"
)
FetchContent_MakeAvailable(curl)
target_link_libraries(usos-rpc libcurl)
message("[usos-rpc] Completed fetching 'libcurl'")


#[===[ date ]===]#
# Date and time library, implementation of C++20 <chrono> header.
set(date_version 8f8336f42be73f4eefb042c3807a5b4399422fed)

set(BUILD_TZ_LIB ON)

if(WIN32)
  set(MANUAL_TZ_DB ON)
  # Fix for MSVC build
  target_compile_definitions(usos-rpc PRIVATE NOMINMAX)
else()
  set(USE_SYSTEM_TZ_DB ON)
endif()

FetchContent_Declare(
  date_src
  GIT_REPOSITORY "https://github.com/HowardHinnant/date.git"
  GIT_TAG "${date_version}"
)
FetchContent_MakeAvailable(date_src)
target_link_libraries(usos-rpc date::date)
target_link_libraries(usos-rpc date::date-tz)
message("[usos-rpc] Completed fetching 'date'")


#[===[ battery-embed ]===]
# Embedding files into the executable.
set(battery_embed_version 1.2.15)

set(B_PRODUCTION_MODE ON)

FetchContent_Declare(
  battery-embed
  GIT_REPOSITORY "https://github.com/batterycenter/embed.git"
  GIT_TAG "v${battery_embed_version}"
)
FetchContent_MakeAvailable(battery-embed)
message("[usos-rpc] Completed fetching 'battery-embed'")

#[===[ tzdata ]===]
# IANA time zone database, needed for 'date' on Windows.
if (WIN32)
  FetchContent_Declare(
    tzdata
    URL "https://www.iana.org/time-zones/repository/tzdata-latest.tar.gz"
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
  )
  FetchContent_MakeAvailable(tzdata)

  file(GLOB tzdata_files "${tzdata_SOURCE_DIR}/*")
  list(FILTER tzdata_files EXCLUDE REGEX "^.+\\/[A-Z]+$")  # All capital letters
  file(COPY ${tzdata_files} DESTINATION "${PROJECT_SOURCE_DIR}/resources/tzdata"
    PATTERN "*.awk" EXCLUDE
    PATTERN "*.html" EXCLUDE
    PATTERN "Makefile" EXCLUDE
  )
  
  # Windows time zones file must be downloaded separately
  file(DOWNLOAD
    "https://raw.githubusercontent.com/unicode-org/cldr/main/common/supplemental/windowsZones.xml"
    "${PROJECT_SOURCE_DIR}/resources/tzdata/windowsZones.xml"
  )
  message("[usos-rpc] Completed fetching 'tzdata'")
endif()

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

# Include info about embedded files in preinit_files.hpp.in
set(usos-rpc_EMBEDDED_FILES "\n")
foreach(resource ${resources})
  if(WIN32)
    # Path separator fix
    string(REPLACE "/" "\\\\" resource_name ${resource})
  else()
    set(resource_name ${resource})
  endif()
  string(APPEND usos-rpc_EMBEDDED_FILES "{\"${resource_name}\", b::embed<\"resources/${resource}\">().data()},\n")
endforeach()
configure_file("src/preinit_files.hpp.in" "preinit_files.hpp")
target_sources(usos-rpc PRIVATE "${PROJECT_BINARY_DIR}/preinit_files.hpp")
message("[usos-rpc] Exported preinit_files.hpp")
