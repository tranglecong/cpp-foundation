project(ThreadSafe)

add_library(ThreadSafe STATIC)

target_sources(ThreadSafe
    PRIVATE
        wait.cpp
        thread.cpp
)

target_include_directories(ThreadSafe 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC
        ${TOP_LEVEL_PROJECT_SOURCE_DIR}
)