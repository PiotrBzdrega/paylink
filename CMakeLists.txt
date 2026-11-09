cmake_minimum_required(VERSION 3.23)
project(paylink VERSION 0.4.0 LANGUAGES C CXX)

option(PAYLINK_BUILD_EXAMPLES "Set to ON to build examples" OFF)

# Add subdirectories
add_subdirectory(lib)

if(PAYLINK_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()