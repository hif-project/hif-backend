# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#85): a Verilog named block and the `disable`
#           that leaves it were dropped, so the loop inside it ran to completion.
#
#           verilog2hif has no node for a named block, so it models one as a named
#           `do { ... } while (false)`, and `disable name` as a Break carrying the
#           same name. VerilogPrinter left both to the guide visitor, which prints
#           the children and nothing else: no `begin : name`, no `disable name;`.
#           A first-set-bit scan then reported the last set bit.
#
#           The fixture is the HIF verilog2hif produces for
#           hif-regression's `combinational/named_block_disable` (source
#           positions removed), so this test needs no front end. The behavioural
#           half is that design's own simulation.
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

if(NOT verilog MATCHES "begin : scan")
    message(FATAL_ERROR "The named block `scan` was not printed (hif-backend#85).\n${verilog}")
endif()

# The disable has to be inside the block it leaves, before its `end`.
if(NOT verilog MATCHES "begin : scan.*disable scan;.*end")
    message(FATAL_ERROR "No `disable scan;` inside the named block (hif-backend#85).\n${verilog}")
endif()

message(STATUS "named_block_disable test passed.")
