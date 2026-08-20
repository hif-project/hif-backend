# -----------------------------------------------------------------------------
# @brief  : Regression: VerilogPrinter::visitSwitch printed the conditions of a
#           SwitchAlt space-separated instead of comma-separated. When two case
#           alternatives share a body they are merged into one alt holding both
#           values, so the emitted label read "STATE_2 FINAL :" instead of
#           "STATE_2, FINAL :" - a syntax error rejected by iverilog, Verilator
#           and verilog2hif alike. See fixtures/merged_case_alternatives.v.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o merged_case_alternatives ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/merged_case_alternatives.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/merged_case_alternatives.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# The merge itself is a legitimate optimisation; what matters is that the
# resulting multi-label alternative is separated by a comma. Match the two
# state names with a comma between them, tolerating whitespace differences.
if(NOT verilog_content MATCHES "STATE_2[ \t]*,[ \t]*FINAL")
    message(FATAL_ERROR "Regenerated Verilog does not comma-separate merged case labels.\nFull content:\n${verilog_content}")
endif()

if(verilog_content MATCHES "STATE_2[ \t]+FINAL")
    message(FATAL_ERROR "Regenerated Verilog still emits space-separated case labels ('STATE_2 FINAL') - the alternative merge regressed.\nFull content:\n${verilog_content}")
endif()

# The real bug manifests as invalid HDL - confirm the regenerated file is
# itself valid input to verilog2hif.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o merged_case_alternatives_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Regenerated Verilog failed to reparse (exit code ${result}) - this is the actual regression.")
endif()

message(STATUS "merged_case_alternatives test passed.")
