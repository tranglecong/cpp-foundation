cmake_minimum_required(VERSION 3.5.0)
project(Foundation VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set a variable for the top-level source directory
set(TOP_LEVEL_PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR})

# Add this line, to enable compile command export
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add subdirectories
add_subdirectory(thread_safe)
add_subdirectory(tests)

