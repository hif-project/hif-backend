# -----------------------------------------------------------------------------
# @brief  : Regression: hif2verilog crashes on HIF with an unresolved
#           module-parameter-width type. See fixtures/unresolved_parameter.v.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o unresolved_parameter ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/unresolved_parameter.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/unresolved_param.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# Must not contain the unsigned-wraparound width literal, or a bare/
# undefined call to the internal iterated_concat system function - both
# previously present, both invalid/nonsensical Verilog.
foreach(unexpected
    "18446744073709551615"
    "hif_verilog_iterated_concat("
)
    string(FIND "${verilog_content}" "${unexpected}" found_at)
    if(NOT found_at EQUAL -1)
        message(FATAL_ERROR "Regenerated Verilog contains invalid content: ${unexpected}\nFull content:\n${verilog_content}")
    endif()
endforeach()

foreach(expected
    "parameter WIDTH = 8"
    "[WIDTH - 1:0]"
    "{WIDTH{1'bz}}"
)
    string(FIND "${verilog_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Regenerated Verilog missing expected content: ${expected}\nFull content:\n${verilog_content}")
    endif()
endforeach()

message(STATUS "unresolved_parameter test passed.")
