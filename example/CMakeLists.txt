set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)  # Enforce strict compliance
set(CMAKE_C_EXTENSIONS OFF)        # Disable compiler-specific extensions

add_executable(${PROJECT_NAME}_example)
target_sources(${PROJECT_NAME}_example PRIVATE main.c)
target_link_libraries(${PROJECT_NAME}_example PRIVATE paylink)