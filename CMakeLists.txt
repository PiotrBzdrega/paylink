cmake_minimum_required(VERSION 3.23)
project(paylink VERSION 0.3.0 LANGUAGES C CXX)

option(PAYLINK_BUILD_EXAMPLES "Set to ON to build examples" ON)

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(example)