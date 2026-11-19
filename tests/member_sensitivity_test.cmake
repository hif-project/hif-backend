# -----------------------------------------------------------------------------
# @brief  : Regression, found in review of the hif-backend#87 fix: a process
#           whose sensitivity list names a bit-select or a part-select crashed
#           hif2verilog.
#
#           The array check added for #87 asked for the declaration of each
#           sensitivity entry. A bit-select (Member) or a part-select (Slice) is
#           not a symbol, and getDeclaration asserts on one ("Passed non-symbol
#           object", the trap of hif-backend#23), so "always @(a[0] or b)" aborted
#           the tool. The entry is now resolved to the identifier it is built on
#           first, and what is not one is skipped.
#
#           The fixture is the HIF verilog2hif produces for
#           "always @(a[0] or b) begin y = a[0] & b; z = a[7]; end" (source
#           positions removed).
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

if(NOT verilog MATCHES "always @\\( a\\[0\\], b \\)")
    message(FATAL_ERROR "The bit-select sensitivity entry was not printed.\n${verilog}")
endif()

message(STATUS "member_sensitivity test passed.")
