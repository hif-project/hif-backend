# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#18): two slice/part-select emission bugs
#           that both produced invalid Verilog while hif2verilog exited 0.
#
#           1. A narrowing Cast of an *expression* was emitted as a
#              part-select appended to that expression. A part-select applies
#              to a net or variable, not to an arbitrary expression, and
#              without parentheses the brackets bound to the shift amount.
#
#           2. getValue() assembles a string, but its Expression branch called
#              the visitor, which writes to the output stream. Slice bounds
#              that were expressions therefore escaped to the output ahead of
#              the string being assembled, emitting every piece of the
#              part-select in the wrong order with empty brackets.
#
#           Both are loud on reparse and silent otherwise, so the check that
#           matters is that the regenerated Verilog is valid input to
#           verilog2hif again.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o slice_emission ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/slice_emission.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/slice_emission.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# Case 2: the part-select must name its prefix first and carry both bounds
# inside the brackets. The pre-fix output had the bounds ahead of the
# identifier and "chain[:]" at the end.
if(NOT verilog_content MATCHES "chain\\[[^]]*:[^]]*\\]")
    message(FATAL_ERROR
        "Part-select on 'chain' is not emitted as chain[<msb>:<lsb>].\nFull content:\n${verilog_content}")
endif()
if(verilog_content MATCHES "chain\\[:\\]")
    message(FATAL_ERROR
        "Part-select bounds escaped the brackets (hif-backend#18).\nFull content:\n${verilog_content}")
endif()

# Case 1: no part-select may be applied to a parenthesised expression - that
# is the shape that is not legal Verilog-2001.
if(verilog_content MATCHES "\\)\\[[0-9]+:[0-9]+\\]")
    message(FATAL_ERROR
        "A part-select is applied to an expression, which is not legal Verilog (hif-backend#18).\nFull content:\n${verilog_content}")
endif()

# The property both cases are really about.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o slice_emission_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}) - this is the actual regression.\nFull content:\n${verilog_content}")
endif()

message(STATUS "slice_emission test passed.")
