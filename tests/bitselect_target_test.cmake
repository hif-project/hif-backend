# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#23): an assignment to a bit-select aborted
#           hif2verilog. VerilogPrinter::visitAssign handed the left-hand side
#           straight to getDeclaration, but a bit-select is a Member wrapping
#           the identifier - not a symbol - so the lookup asserted inside
#           hif-core ("Passed non-symbol object"). The process terminated
#           after the output file had been created, leaving it zero bytes.
#
#           Note the explicit non-empty check below. Reparsing is not
#           sufficient evidence here: an empty file is valid Verilog and
#           reparses cleanly, so a round-trip check alone would have scored
#           the broken output as a pass.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o bitselect_target ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/bitselect_target.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "hif2verilog failed with exit code ${result} (expected 0).\nTool output:\n${tool_output}")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/bitselect_target.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

# The actual failure mode: the file exists but is empty.
file(SIZE ${OUTPUT_VERILOG} output_size)
if(output_size EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog is zero bytes - hif2verilog created the file and then aborted "
        "(hif-backend#23).\nTool output:\n${tool_output}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

if(NOT verilog_content MATCHES "endmodule")
    message(FATAL_ERROR "Regenerated Verilog has no module body.\nFull content:\n${verilog_content}")
endif()

# Every bit-select target must survive as an assignment target.
foreach(expected "y[0]" "y[1]" "z[0]" "z[1]" "b[0]" "b[3]")
    string(FIND "${verilog_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Regenerated Verilog is missing bit-select target ${expected}.\nFull content:\n${verilog_content}")
    endif()
endforeach()

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o bitselect_target_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

message(STATUS "bitselect_target test passed.")
