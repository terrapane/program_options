cmake_minimum_required(VERSION 3.21)

# Define the Program Options library
project(program_options
        VERSION 1.0.0.0
        DESCRIPTION "Program Options Library"
        LANGUAGES CXX)

# Tests are built by default when this is a top-level project
if(PROJECT_IS_TOP_LEVEL)
    # Option to control whether tests are built
    option(program_options_BUILD_TESTS "Build Tests for the Program Options Library" ON)
else()
    # Option to control whether tests are built
    option(program_options_BUILD_TESTS "Build Tests for the Program Options Library" OFF)
endif()

# Option to control ability to install the library
option(program_options_INSTALL "Install the Program Options Library" ON)

add_subdirectory(dependencies)
add_subdirectory(src)

include(CTest)

if(BUILD_TESTING AND program_options_BUILD_TESTS)
    add_subdirectory(test)
    add_subdirectory(sample)
endif()
