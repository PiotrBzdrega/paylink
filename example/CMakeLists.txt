set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)  # Enforce strict compliance
set(CMAKE_C_EXTENSIONS OFF)        # Disable compiler-specific extensions

add_executable(paylink_example)
target_sources(paylink_example PRIVATE main.c)
target_link_libraries(paylink_example PRIVATE paylink)