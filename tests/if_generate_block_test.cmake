# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#86): an if generate was printed without its
#           generate block.
#
#           VerilogPrinter left IfGenerate to the guide visitor, which printed
#           the condition bare ahead of the first process in the branch -
#           `WIDE[0:0]always @( a, b ) begin` - and dropped the block, its name
#           and the generate keywords. The file did not parse.
#
#           Each IfGenerate is now `generate if (cond) begin : name ... end
#           endgenerate`. The `else` of the source arrives as a second
#           IfGenerate with the negated condition, so both branches are checked,
#           and the process must sit inside its block.
#
#           The fixture is the HIF verilog2hif produces for a two-branch static
#           if generate (source positions removed). hif-regression's
#           `parameterized/if_generate` is the structural half.
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

foreach(branch wide_path narrow_path)
    if(NOT verilog MATCHES "generate[^g]*if \\(.*\\) begin : ${branch}.*always.*endgenerate")
        message(FATAL_ERROR
            "Branch '${branch}' is not printed as a generate block containing its process (hif-backend#86).\n${verilog}")
    endif()
endforeach()

# The condition must not be left bare ahead of a process.
if(verilog MATCHES "\\]always")
    message(FATAL_ERROR "A condition was printed bare ahead of a process (hif-backend#86).\n${verilog}")
endif()

message(STATUS "if_generate_block test passed.")
