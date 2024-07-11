# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
# functions.cmake for Simple-XX/SimpleRenderer. Helper functions.

# Function to add a coverage target
function(add_coverage_target)
    # Parse arguments
    set(options)
    set(one_value_keywords SOURCE_DIR BINARY_DIR)
    set(multi_value_keywords DEPENDS EXCLUDE_DIR)
    cmake_parse_arguments(ARG "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})

    # Exclude directories
    list(APPEND EXCLUDES --exclude)
    foreach (_item ${ARG_EXCLUDE_DIR})
        list(APPEND EXCLUDES "${_item}")
    endforeach()

    # Add coverage target
    add_custom_target(coverage DEPENDS ${ARG_DEPENDS}
        COMMAND ${CMAKE_CTEST_COMMAND}
    )

    # Generate coverage report after running tests
    add_custom_command(TARGET coverage
        COMMENT "Generating coverage report ..."
        POST_BUILD
        WORKING_DIRECTORY ${ARG_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_DIR}
        COMMAND ${LCOV_EXE}
        -c
        -o ${COVERAGE_OUTPUT_DIR}/coverage.info
        -d ${ARG_BINARY_DIR}
        -b ${ARG_SOURCE_DIR}
        --no-external
        ${EXCLUDES}
        --rc lcov_branch_coverage=1
        COMMAND ${GENHTML_EXE}
        ${COVERAGE_OUTPUT_DIR}/coverage.info
        -o ${COVERAGE_OUTPUT_DIR}
        --branch-coverage
    )
endfunction()
