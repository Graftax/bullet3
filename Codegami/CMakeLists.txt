cmake_minimum_required(VERSION 3.10)

project(Codegami)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_compile_definitions(_REGEX_MAX_STACK_COUNT=0)

# Add the source files to the project
set(SOURCE_FILES
	main.cpp)

add_executable(Codegami ${SOURCE_FILES})