# -----------------------------------------------------------------------------
# @brief  : Regression: hif2verilog renders a top-level, uninstantiated
#           module's parametric port width as a huge unsigned-wraparound
#           literal (e.g. [18446744073709551615:0], i.e. 2^64-1) instead of
#           the symbolic width expression, and separately never prints the
#           parameter's own declaration at all. See
#           fixtures/parametric_port_width.v.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o parametric_port_width ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/parametric_port_width.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/parametric_port_width.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# Must NOT contain the unsigned-wraparound literal.
string(FIND "${verilog_content}" "18446744073709551615" found_wraparound)
if(NOT found_wraparound EQUAL -1)
    message(FATAL_ERROR "Regenerated Verilog contains the unsigned-wraparound width literal:\n${verilog_content}")
endif()

foreach(expected
    "parameter WIDTH = 8"
    "[WIDTH - 1:0]"
)
    string(FIND "${verilog_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Regenerated Verilog missing expected content: ${expected}\nFull content:\n${verilog_content}")
    endif()
endforeach()

message(STATUS "parametric_port_width test passed.")
