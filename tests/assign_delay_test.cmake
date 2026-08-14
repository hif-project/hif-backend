# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#24): the delay on a delayed assignment was
#           dropped. The HIF carried it - verilog2hif records `assign #2` as an
#           explicit DELAY, scaled by the timescale unit - and visitAssign never
#           read it, so the regenerated design responded immediately where its
#           source waited.
#
#           Silent: hif2verilog exited 0 and the output both parsed and
#           reparsed, so only simulation with timing can see it. The trace
#           comparison is therefore the test; the text checks below exist to
#           make a failure point at the cause rather than only at a mismatch.
#
#           Also covers the `timescale that has to come with the delay. A delay
#           is a plain number counting timescale units, so emitting one means
#           emitting a matching directive - and the two timescale constants
#           verilog2hif records must stop being printed as valueless
#           `localparam`s, which no simulator accepted.
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

# Simulate a source together with the shared testbench and return its trace.
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

    string(STRIP "${trace}" trace)
    set(${out_trace} "${trace}" PARENT_SCOPE)
endfunction()

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o assign_delay ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/assign_delay.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The delay must be in the HIF for this test to be about emission at all.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "<DELAY>")
    message(FATAL_ERROR
        "verilog2hif did not record the assignment delay, so this test is not exercising the emission gap it "
        "is about. The frontend, not hif2verilog, would be at fault.")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/assign_delay.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# The delay itself.
if(NOT verilog_content MATCHES "#2 ")
    message(FATAL_ERROR
        "Regenerated Verilog carries no '#2' delay; the HIF's DELAY was dropped (hif-backend#24).\nFull content:\n${verilog_content}")
endif()

# The unit it counts in. Without the directive the number would mean whatever
# the consumer's default timescale happens to be.
if(NOT verilog_content MATCHES "`timescale 1ns / 1ps")
    message(FATAL_ERROR
        "Regenerated Verilog does not restate the source's `timescale, so its delay counts an undefined "
        "unit (hif-backend#24).\nFull content:\n${verilog_content}")
endif()

# The timescale is a directive, not a declaration. These used to be emitted as
# "localparam hif_verilog_timescale_unit;" - no value, which Icarus rejects and
# verilog2hif cannot reparse.
if(verilog_content MATCHES "hif_verilog_timescale")
    message(FATAL_ERROR
        "Regenerated Verilog declares the timescale bookkeeping constants instead of only stating the "
        "directive; they have no Verilog literal, so the file does not compile.\nFull content:\n${verilog_content}")
endif()

# Reparsing must still work - the fix adds new text to the output.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o assign_delay_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# The property all of the above is really about.
simulate(original ${FIXTURE} original_trace)
simulate(regenerated ${OUTPUT_VERILOG} regenerated_trace)

if(NOT original_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated Verilog does not reproduce the source's timing (hif-backend#24).\n"
        "--- original trace ---\n${original_trace}\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

# A trace of nothing but x would compare equal to itself and prove nothing.
if(original_trace MATCHES "y=x" OR original_trace MATCHES "z=x")
    message(FATAL_ERROR
        "Testbench never resolved the original design's outputs, so the comparison is vacuous:\n${original_trace}")
endif()

# And the trace has to actually distinguish before-delay from after-delay, or
# an implementation that ignored the delay would match it too.
if(NOT original_trace MATCHES "a,b rise \\+1  y=0" OR NOT original_trace MATCHES "a,b rise \\+3  y=1")
    message(FATAL_ERROR
        "The reference trace does not show the delay taking effect between +1 and +3, so it cannot "
        "distinguish a honoured delay from a dropped one:\n${original_trace}")
endif()

message(STATUS "assign_delay test passed.")
