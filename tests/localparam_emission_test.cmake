# -----------------------------------------------------------------------------
# @brief  : Regression: hif2verilog silently dropped `localparam`
#           declarations (HIF Const) - VerilogPrinter::getDeclaration() had
#           no branch for Const, returned an empty string, and printList()
#           then silently skips any item whose declaration string is empty
#           (no output, no separator, no diagnostic). Code referencing the
#           constant (e.g. a state machine's state encoding) kept compiling
#           and even reparsing - this is a *semantic* loss, not a syntax
#           error, so a reparse-only check would miss it entirely. See
#           fixtures/localparam_emission.v.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o localparam_emission ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/localparam_emission.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/localparam_emission.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# The actual regression: the regenerated source must declare the constants
# it uses, not just happen to reparse without them.
foreach(expected
    "localparam IDLE"
    "localparam RUN"
)
    string(FIND "${verilog_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Regenerated Verilog missing expected declaration: ${expected}\nFull content:\n${verilog_content}")
    endif()
endforeach()

message(STATUS "localparam_emission test passed.")
