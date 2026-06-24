cmake_minimum_required(VERSION 3.23)

if(NOT PAYLINK_BUILD_EXAMPLES)
  return()
endif()

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v6.1.9  # Replace with a version, tag, or commit hash
)

FetchContent_MakeAvailable(ftxui)

add_executable(${PROJECT_NAME}_example)
target_sources(${PROJECT_NAME}_example PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME}_example 
    PRIVATE 
        paylink
        ftxui::screen
        ftxui::dom
        ftxui::component)