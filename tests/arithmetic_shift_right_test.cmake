# -----------------------------------------------------------------------------
# @brief  : Regression: VerilogPrinter::visitExpression printed op_sra as the
#           literal token "sra" - HIF's internal operator name - instead of
#           Verilog's ">>>", so the regenerated file did not parse. The
#           logical shifts in the same switch were already correct, and the
#           fixture carries a ">>" alongside as a control. See
#           fixtures/arithmetic_shift_right.v.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o arithmetic_shift_right ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/arithmetic_shift_right.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/arithmetic_shift_right.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# The operator is printed surrounded by single spaces, so anchoring on
# " >>> " matches the emitted token rather than any substring of an
# identifier.
set(expected " >>> ")
string(FIND "${verilog_content}" "${expected}" found_at)
if(found_at EQUAL -1)
    message(FATAL_ERROR "Regenerated Verilog missing the arithmetic right shift operator '>>>'.\nFull content:\n${verilog_content}")
endif()

set(broken " sra ")
string(FIND "${verilog_content}" "${broken}" found_at)
if(NOT found_at EQUAL -1)
    message(FATAL_ERROR "Regenerated Verilog still contains the HIF operator name 'sra' - the arithmetic right shift regressed.\nFull content:\n${verilog_content}")
endif()

# The control: the logical shift shares the switch and must be untouched.
set(control " >> ")
string(FIND "${verilog_content}" "${control}" found_at)
if(found_at EQUAL -1)
    message(FATAL_ERROR "Regenerated Verilog missing the logical right shift operator '>>' - the control operand regressed.\nFull content:\n${verilog_content}")
endif()

# The real bug manifests as a reparse failure - confirm the regenerated file
# is itself valid input to verilog2hif.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o arithmetic_shift_right_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Regenerated Verilog failed to reparse (exit code ${result}) - this is the actual regression.")
endif()

message(STATUS "arithmetic_shift_right test passed.")
