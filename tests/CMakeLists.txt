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
    # Regression: hif2verilog crashes printing op_andrd/op_orrd (the
    # reduction-AND/OR operators used internally to implement ~& and ~|).
    add_test(
        NAME reduction_operators
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/reduction_operators.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/reduction_operators_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/reduction_operators_test.cmake
    )
    # Regression: hif2verilog renders an unresolved top-level parametric
    # port width as a huge unsigned-wraparound literal, and never prints
    # the parameter's own declaration at all.
    add_test(
        NAME parametric_port_width
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/parametric_port_width.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/parametric_port_width_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/parametric_port_width_test.cmake
    )
    # Regression: hif2verilog silently dropped Procedure declarations (the
    # "cone functions" hif-frontend synthesizes for primitive gate
    # instances and flattened combinational submodule instances) instead
    # of printing their driving logic.
    add_test(
        NAME gate_primitives
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/gate_primitives.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/gate_primitives_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/gate_primitives_test.cmake
    )
    # Same root cause as gate_primitives, from plain module instantiation.
    add_test(
        NAME module_instantiation
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/module_instantiation.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/module_instantiation_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/module_instantiation_test.cmake
    )
    # Regression: concatenation printed as "a { } b" (an infix operator
    # token) instead of "{a, b}".
    add_test(
        NAME concat_emission
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/concat_emission.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/concat_emission_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/concat_emission_test.cmake
    )
    # Regression: `localparam` declarations (HIF Const) were silently
    # dropped from regenerated Verilog.
    add_test(
        NAME localparam_emission
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/localparam_emission.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/localparam_emission_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/localparam_emission_test.cmake
    )
else()
    message(STATUS "verilog2hif not found; skipping unresolved_parameter/reduction_operators/parametric_port_width/gate_primitives/module_instantiation/concat_emission/localparam_emission tests.")
endif()
