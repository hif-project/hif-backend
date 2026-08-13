# -----------------------------------------------------------------------------
# @brief  : Regression: VerilogPrinter::visitExpression treated concatenation
#           (op_concat) as a plain infix operator, printing the literal
#           token "{ }" between the two operands instead of wrapping them -
#           "a { } b" instead of "{a, b}". See fixtures/concat_emission.v.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o concat_emission ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/concat_emission.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/concat_emission.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

set(expected "{a, b}")
string(FIND "${verilog_content}" "${expected}" found_at)
if(found_at EQUAL -1)
    message(FATAL_ERROR "Regenerated Verilog missing expected content: ${expected}\nFull content:\n${verilog_content}")
endif()

set(broken "{ }")
string(FIND "${verilog_content}" "${broken}" found_at)
if(NOT found_at EQUAL -1)
    message(FATAL_ERROR "Regenerated Verilog still contains the broken '{ }' token - concatenation regressed.\nFull content:\n${verilog_content}")
endif()

# The real bug manifests as a reparse failure - confirm the regenerated file
# is itself valid input to verilog2hif.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o concat_emission_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Regenerated Verilog failed to reparse (exit code ${result}) - this is the actual regression.")
endif()

message(STATUS "concat_emission test passed.")
