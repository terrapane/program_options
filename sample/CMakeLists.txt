# Create the sample program
add_executable(program_options_sample sample.cpp)

# Link against the ProgramOptions library
target_link_libraries(program_options_sample PRIVATE program_options)

# Specify the C++ standard to observe
set_target_properties(program_options_sample
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

# Use the following compile options
target_compile_options(program_options_sample
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
            $<$<CXX_COMPILER_ID:MSVC>: >)
