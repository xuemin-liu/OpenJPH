# CMakeLists.txt for ojph_wrapper DLL project

# Explicitly list source files for better control
set(WRAPPER_SOURCES
    ojph_wrapper.cpp
    Htj2kProcessor.cpp
)

set(WRAPPER_HEADERS
    ojph_wrapper.h
    Htj2kProcessor.h
)

# Collect any additional source files with glob as fallback
file(GLOB ADDITIONAL_SOURCES "*.cpp" "*.c")
file(GLOB ADDITIONAL_HEADERS "*.h" "*.hpp")

# Remove already listed files from additional files to avoid duplicates
list(REMOVE_ITEM ADDITIONAL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ojph_wrapper.cpp)
list(REMOVE_ITEM ADDITIONAL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Htj2kProcessor.cpp)
list(REMOVE_ITEM ADDITIONAL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/ojph_wrapper.h)
list(REMOVE_ITEM ADDITIONAL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/Htj2kProcessor.h)

# Combine all sources and headers
set(ALL_SOURCES ${WRAPPER_SOURCES} ${ADDITIONAL_SOURCES})
set(ALL_HEADERS ${WRAPPER_HEADERS} ${ADDITIONAL_HEADERS})

# Create the DLL target
add_library(ojph_wrapper SHARED ${ALL_SOURCES} ${ALL_HEADERS})

# Link with the main openjph library
target_link_libraries(ojph_wrapper PRIVATE openjph)

# Include directories
target_include_directories(ojph_wrapper PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/common
)

# Configure for DLL export
target_compile_definitions(ojph_wrapper PRIVATE 
    OJPH_WRAPPER_EXPORTS
    OJPH_BUILD_SHARED_LIBRARY
)

# Set output name and properties
set_target_properties(ojph_wrapper PROPERTIES
    OUTPUT_NAME "ojph_wrapper$<$<CONFIG:Debug>:d>"
    VERSION ${OPENJPH_VERSION}
    SOVERSION ${OPENJPH_VERSION_MAJOR}
)

# For Windows, ensure static runtime even for DLL
if(MSVC)
    # Override runtime library to static for this DLL
    set_target_properties(ojph_wrapper PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
    
    # Also set the flags explicitly
    target_compile_options(ojph_wrapper PRIVATE
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:Debug>:/MTd>
    )
    
    # Export symbols
    target_compile_definitions(ojph_wrapper PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
endif()

# Install the DLL
install(TARGETS ojph_wrapper
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install headers if any
if(ALL_HEADERS)
    install(FILES ${ALL_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ojph_wrapper
    )
endif()

# Create a simple .def file for clean exports
if(WIN32 AND MSVC)
    set(DEF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ojph_wrapper.def")
    if(EXISTS ${DEF_FILE})
        set_target_properties(ojph_wrapper PROPERTIES
            LINK_FLAGS "/DEF:${DEF_FILE}"
        )
    endif()
endif()

# Group files in IDE
source_group("Source Files\\C Wrapper" FILES ojph_wrapper.cpp)
source_group("Source Files\\C++ Interface" FILES Htj2kProcessor.cpp)
source_group("Source Files\\Additional" FILES ${ADDITIONAL_SOURCES})
source_group("Header Files\\C Wrapper" FILES ojph_wrapper.h)
source_group("Header Files\\C++ Interface" FILES Htj2kProcessor.h)
source_group("Header Files\\Additional" FILES ${ADDITIONAL_HEADERS})
source_group("Export Files" FILES ojph_wrapper.def)

# Print configuration info
message(STATUS "ojph_wrapper DLL will be built with static runtime linking")
message(STATUS "ojph_wrapper includes C wrapper and C++ interface (Htj2kProcessor)")
