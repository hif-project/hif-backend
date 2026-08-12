# -----------------------------------------------------------------------------
# @brief  : Tests cmake file.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

# This regression needs verilog2hif (a sibling repo's tool, not a build
# dependency of this one) to produce its input HIF. Look on PATH and, since
# this project is normally checked out alongside its siblings, in
# hif-frontend's build directory too.
find_program(
    VERILOG2HIF_EXECUTABLE
    NAMES verilog2hif
    HINTS ${CMAKE_SOURCE_DIR}/../hif-frontend/build
)

if(VERILOG2HIF_EXECUTABLE)
    add_test(
        NAME unresolved_parameter
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/unresolved_parameter.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/unresolved_parameter_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/unresolved_parameter_test.cmake
    )
else()
    message(STATUS "verilog2hif not found; skipping unresolved_parameter test.")
endif()
