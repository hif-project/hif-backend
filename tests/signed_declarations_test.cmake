# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#81): `signed` was dropped from declarations
#           and a signed operand was widened by zero-padding.
#
#           Verilog carries signedness on the declaration, and it decides how an
#           operand is extended and how two are compared. With `signed` gone,
#           -3 + 2 in a 5-bit result read 15 and every comparison with exactly
#           one negative operand was unsigned.
#
#           Two things are checked: the ports keep `signed`, and a widening of a
#           signed operand replicates its sign bit rather than padding zeros.
#
#           The fixture is the HIF verilog2hif produces for a small signed adder
#           and comparator (source positions removed). The behavioural half is
#           hif-regression's `combinational/signed_add` and `signed_compare`.
# -----------------------------------------------------------------------------

foreach(required HIF2VERILOG_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${FIXTURE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result}.\n${tool_output}")
endif()

file(GLOB_RECURSE outputs ${WORK_DIR}/verilog_out/*.v)
list(LENGTH outputs output_count)
if(NOT output_count EQUAL 1)
    message(FATAL_ERROR "Expected one Verilog file, found ${output_count}: ${outputs}")
endif()
file(READ ${outputs} verilog)

foreach(port "input wire signed \\[3:0\\] a" "input wire signed \\[3:0\\] b" "output reg signed \\[4:0\\] wide")
    if(NOT verilog MATCHES "${port}")
        message(FATAL_ERROR "Port lost its `signed` qualifier: expected '${port}' (hif-backend#81).\n${verilog}")
    endif()
endforeach()

# Sign extension of a 4-bit signed operand to 5 bits: replicate the sign bit.
if(NOT verilog MATCHES "{{1{a\\[3\\]}}, a}")
    message(FATAL_ERROR "Signed operand `a` is not sign-extended (hif-backend#81).\n${verilog}")
endif()
if(verilog MATCHES "{1'b0, a}")
    message(FATAL_ERROR "Signed operand `a` is zero-extended (hif-backend#81).\n${verilog}")
endif()

message(STATUS "signed_declarations test passed.")
