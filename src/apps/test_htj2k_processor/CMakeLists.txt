# CMakeLists.txt for test_htj2k_processor

# Define the executable
add_executable(test_htj2k_processor
    test_htj2k_processor.cpp
    Htj2kProcessor.cpp
)

# Link the necessary libraries
target_link_libraries(test_htj2k_processor PRIVATE openjph)

# Include directories
target_include_directories(test_htj2k_processor PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/apps/test_htj2k_processor
)

# Set C++ standard
set_target_properties(test_htj2k_processor PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Add this project to the test suite if tests are enabled
if (OJPH_BUILD_TESTS)
    add_test(NAME test_htj2k_processor COMMAND test_htj2k_processor)
endif()
