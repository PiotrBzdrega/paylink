cmake_minimum_required(VERSION 3.10.0)
project(paylink_app VERSION 0.1.0 LANGUAGES C CXX)

# Set C++23 standard for all targets
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Enforce strict compliance
set(CMAKE_CXX_EXTENSIONS OFF)        # Disable compiler-specific extensions

# Configure the version header file
configure_file(version.h.in "${CMAKE_SOURCE_DIR}/include/version.h")

find_path(AES_LIB_INCLUDE Aesimhei.h REQUIRED)

find_library(AES_LIB NAMES aes_access REQUIRED VERSION 1.2.3)

# Ensure both the library and header were found
if (NOT AES_LIB_INCLUDE)
  message(FATAL_ERROR "Aesimhei.h not found!")
endif()
message(STATUS "lib include path:${AES_LIB_INCLUDE}")

if (NOT AES_LIB)
  message(FATAL_ERROR "AES_LIB not found!")
endif()
message(STATUS "lib path=${AES_LIB}")

# Include directories for headers
include_directories("${CMAKE_SOURCE_DIR}/include")

# Specify the directory containing your source files
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    m
    rt
    aes_access
    stdc++
)

# Set compiler options for this target only
target_compile_options(${PROJECT_NAME} PRIVATE
    -O2
    -g0
    -Wall
    -Wextra
    -Wpedantic
    -pedantic
    # -Wstrict-prototypes
    -pipe
    -fomit-frame-pointer
)

target_compile_definitions(${PROJECT_NAME} PRIVATE _GNU_SOURCE)