# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#95): the arithmetic left shift was printed as
#           the HIF operator name, "sla", which is not Verilog. The right shift
#           already printed as ">>>"; the left one is "<<<".
#
#           The fixture is the HIF verilog2hif produces for a module shifting a
#           signed value right and left arithmetically and an unsigned one right
#           (source positions removed).
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

if(verilog MATCHES " sla ")
    message(FATAL_ERROR "The arithmetic left shift was printed as the HIF operator name (hif-backend#95).\n${verilog}")
endif()
if(NOT verilog MATCHES "<<<")
    message(FATAL_ERROR "No arithmetic left shift in the output (hif-backend#95).\n${verilog}")
endif()

message(STATUS "arithmetic_left_shift test passed.")
