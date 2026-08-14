# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#30): a constant continuous assignment is
#           folded by verilog2hif into the driven port's initial value, and
#           hif2verilog printed that value nowhere. Verilog-2001 has no place
#           for an initializer inside an ANSI port list, so the assignment was
#           simply lost and the module regenerated with an undriven output.
#
#           Silent: exit code 0, and the output both compiled and reparsed. The
#           only visible symptom is that the output reads x, so simulation is
#           the test. The text checks below exist to make a failure point at
#           the cause rather than only at a mismatch.
#
#           The fix re-emits the folded value as the continuous assignment it
#           came from, which also means the port has to become a net - the same
#           rule hif-backend#26 introduced and hif-backend#32 generalized.
#
#           Deliberately NOT covered here: an output port carrying an initial
#           value while a *process* drives it. There a continuous assign would
#           be a second driver and the correct form is an initial block, which
#           is a different decision. Tracked as hif-backend#36.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE VVP_EXECUTABLE
                 FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

function(simulate label source out_trace)
    set(vvp_image ${WORK_DIR}/${label}.vvp)
    execute_process(
        COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${vvp_image} ${source} ${TESTBENCH}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE compile_output
        ERROR_VARIABLE compile_output
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "iverilog failed to compile ${label} (${source}), exit code ${result}:\n${compile_output}")
    endif()

    execute_process(
        COMMAND ${VVP_EXECUTABLE} ${vvp_image}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE trace
        ERROR_VARIABLE run_errors
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "vvp failed to run ${label}, exit code ${result}:\n${run_errors}")
    endif()

    string(REPLACE "\r\n" "\n" trace "${trace}")
    string(REGEX REPLACE "\n[^\n]*\\$finish[^\n]*" "" trace "${trace}")
    string(STRIP "${trace}" trace)
    set(${out_trace} "${trace}" PARENT_SCOPE)
endfunction()

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o port_initial_value ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/port_initial_value.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The fold has to have happened for this test to be about emission at all. If
# the frontend ever stops folding a constant assignment into the port value,
# these assignments arrive as a GlobalAction instead and this test would be
# silently exercising hif-backend#32's path rather than this one.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "<PORT direction=\"OUT\"[^>]*>[ \t\r\n]*<VALUE>")
    message(FATAL_ERROR
        "verilog2hif did not fold the constant continuous assignments into the output ports' values, so this "
        "test is not exercising the emission gap it is about.")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/port_initial_value.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The drivers exist at all. ----------------------------------------------
foreach(port c m b)
    if(NOT verilog_content MATCHES "assign ${port} =")
        message(FATAL_ERROR
            "Regenerated Verilog has no driver for output '${port}': the port's folded initial value was "
            "dropped (hif-backend#30).\nFull content:\n${verilog_content}")
    endif()
endforeach()

# --- The parameterized one has to survive as an expression, not be -----------
# --- constant-folded away or dropped for not being a literal. ---------------
if(NOT verilog_content MATCHES "{N{1'b1}}")
    message(FATAL_ERROR
        "Regenerated Verilog lost the parameterized replication driving 'm'; the folded value is an "
        "expression rather than a literal and has to be re-emitted as one (hif-backend#30).\n"
        "Full content:\n${verilog_content}")
endif()

# --- What a continuous assign drives must be a net (hif-backend#26/#32). ----
foreach(port c m b)
    if(NOT verilog_content MATCHES "output wire[^;,\n]*${port}")
        message(FATAL_ERROR
            "Output port '${port}' is driven by a continuous assign but was not declared as 'output wire'; "
            "Verilog forbids a continuous assign to a reg.\nFull content:\n${verilog_content}")
    endif()
endforeach()
if(verilog_content MATCHES "output reg")
    message(FATAL_ERROR
        "Regenerated Verilog still declares a continuously driven output as reg.\n"
        "Full content:\n${verilog_content}")
endif()

# --- Reparse. ---------------------------------------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o port_initial_value_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- The property all of the above is really about. -------------------------
simulate(original ${FIXTURE} original_trace)
simulate(regenerated ${OUTPUT_VERILOG} regenerated_trace)

if(NOT original_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated Verilog does not reproduce the source's outputs (hif-backend#30).\n"
        "--- original trace ---\n${original_trace}\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

# An undriven output would compare equal to itself if both sides were broken,
# so the trace has to be independently checked for having driven anything. The
# testbench reports this per port rather than only printing values, so that a
# failure distinguishes "wrong value" from "no driver".
if(regenerated_trace MATCHES "driven=0")
    message(FATAL_ERROR
        "An output is undriven in the regenerated design (reads x), which is exactly hif-backend#30.\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

# And the reference has to be non-vacuous in the same way.
if(original_trace MATCHES "driven=0")
    message(FATAL_ERROR
        "The fixture's own outputs are undriven, so the comparison proves nothing:\n${original_trace}")
endif()

message(STATUS "port_initial_value test passed.")
