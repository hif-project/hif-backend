# -----------------------------------------------------------------------------
# @brief  : Regression: hif2verilog silently dropped Procedure declarations
#           (the "cone functions" hif-frontend synthesizes for primitive gate
#           instances - see FixDescription_3.cpp's generateConeFunctions),
#           emitting a bare ';' instead of a driver, and producing
#           syntactically broken Verilog. See fixtures/gate_primitives.v.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o gate_primitives ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/gate_primitives.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/gate_primitives.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

foreach(expected
    "ab = a & b"
    "axb = a ^ b"
    "axb_cin = axb & cin"
)
    string(FIND "${verilog_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Regenerated Verilog missing expected content: ${expected}\nFull content:\n${verilog_content}")
    endif()
endforeach()

# Require a cone's driver to be emitted inside the very block that consumes
# it, with no intervening `always` opening a new one.
#
# This replaces an earlier check for the literal string "always @(*)". That
# string pinned the shape of the fix that first made cone logic appear at
# all - each cone hoisted into a combinational block of its own - rather
# than the property being tested, which is that the driver is emitted and
# reaches its consumer. Hoisting turned out to be what made regenerated
# Verilog simulate differently from its source (hif-backend#16): the
# consumer was left sensitive to the cone's primary inputs while reading a
# target another block wrote. The check below is strictly stronger - it
# would have failed on the hoisted output that "always @(*)" accepted.
function(require_driver_in_consumer_block driver consumer)
    string(FIND "${verilog_content}" "${driver}" driver_at)
    if(driver_at EQUAL -1)
        message(FATAL_ERROR "Regenerated Verilog missing cone driver: ${driver}\nFull content:\n${verilog_content}")
    endif()
    string(FIND "${verilog_content}" "${consumer}" consumer_at)
    if(consumer_at EQUAL -1)
        message(FATAL_ERROR "Regenerated Verilog missing consumer: ${consumer}\nFull content:\n${verilog_content}")
    endif()
    if(driver_at GREATER consumer_at)
        message(FATAL_ERROR
            "Cone driver '${driver}' is emitted after its consumer '${consumer}'.\nFull content:\n${verilog_content}")
    endif()
    math(EXPR span "${consumer_at} - ${driver_at}")
    string(SUBSTRING "${verilog_content}" ${driver_at} ${span} between)
    if(between MATCHES "always")
        message(FATAL_ERROR
            "Cone driver '${driver}' is hoisted into a separate always block from its consumer "
            "'${consumer}', so the consumer is not sensitive to it (hif-backend#16).\nFull content:\n${verilog_content}")
    endif()
endfunction()

require_driver_in_consumer_block("axb = a ^ b" "sum <= axb ^ cin")
require_driver_in_consumer_block("ab = a & b" "cout <= ab | axb_cin")
require_driver_in_consumer_block("axb_cin = axb & cin" "cout <= ab | axb_cin")

# The bug this guards against actually manifests as a reparse failure (the
# dropped Procedure declarations became bare ';' statements - a syntax
# error), not just missing text. Confirm the regenerated file is itself
# valid input to verilog2hif.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o gate_primitives_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Regenerated Verilog failed to reparse (exit code ${result}) - this is the actual regression.")
endif()

message(STATUS "gate_primitives test passed.")
