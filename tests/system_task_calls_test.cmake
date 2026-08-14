# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#29): hif2verilog emitted nothing at all for
#           a Verilog system *task* call. hif-core declares $display and its
#           neighbours as subprograms with no return type and no body, so the
#           call resolved to a Procedure whose StateTable is null and
#           visitProcedureCall's "nothing to inline" early return dropped it
#           silently. Exit 0, and the output parsed and reparsed cleanly.
#
#           The regenerated design compiled and simulated and printed nothing:
#           a round trip discarded the whole observable output of a testbench
#           or an instrumented model.
#
#           Also covers string literals, which visitStringValue never printed.
#           That is not a separate concern here: restoring $display without it
#           yields "$display(, a)", which has lost the format string carrying
#           the message and which verilog2hif rejects on the way back in, so a
#           fix that re-emits system task calls has to own how their arguments
#           are written.
#
#           And it covers the cone-inlining path this shares. visitProcedureCall
#           is also what expands frontend-synthesized logic cones at their call
#           sites, carrying the hif-backend#16 contract, so the fixture holds
#           gate primitives too and this test requires the inlined cone bodies
#           to still be there. cone_signal_target guards the contract itself.
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
    string(REGEX REPLACE "\n?[^\n]*\\$finish[^\n]*" "" trace "${trace}")
    string(STRIP "${trace}" trace)
    set(${out_trace} "${trace}" PARENT_SCOPE)
endfunction()

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o system_task_calls ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/system_task_calls.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The calls must be in the HIF for this test to be about emission at all.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "PCALL name=\"hif_verilog__system_display\"")
    message(FATAL_ERROR
        "verilog2hif did not record the $display calls, so this test is not exercising the emission gap it is "
        "about. The frontend, not hif2verilog, would be at fault.")
endif()
# And so must the cones, or the "they coexist" half of this test is vacuous.
if(NOT hif_content MATCHES "PCALL name=\"hif_cone_")
    message(FATAL_ERROR
        "verilog2hif synthesized no cone procedures for the gate primitives, so this test cannot show that "
        "system task calls and cone inlining coexist.")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/system_task_calls.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The calls exist at all, under their '$' spelling. ----------------------
if(NOT verilog_content MATCHES "\\$display")
    message(FATAL_ERROR
        "Regenerated Verilog contains no $display: the system task call was dropped (hif-backend#29).\n"
        "Full content:\n${verilog_content}")
endif()
if(NOT verilog_content MATCHES "\\$write")
    message(FATAL_ERROR
        "Regenerated Verilog contains no $write, so only one system task is covered (hif-backend#29).\n"
        "Full content:\n${verilog_content}")
endif()

# The internal name must not survive: no simulator accepts it and the frontend
# cannot bind it on the way back in. This is the task-side counterpart of the
# check system_function_round_trip makes for functions (hif-backend#19).
if(verilog_content MATCHES "_system_")
    message(FATAL_ERROR
        "Regenerated Verilog still carries the internal '_system_' name rather than the '$' spelling.\n"
        "Full content:\n${verilog_content}")
endif()

# --- Arguments, including the string literal that carries the message. ------
if(NOT verilog_content MATCHES "\\$display\\(\"a=%b b=%b\", a, b\\)")
    message(FATAL_ERROR
        "Regenerated $display lost its format string or its arguments. Emitting the call alone produces "
        "'$display(, a)', which says nothing and does not reparse (hif-backend#29).\n"
        "Full content:\n${verilog_content}")
endif()

# Escapes are stored by verilog2hif in source form, so they must be written
# back out unchanged. Re-escaping would double every backslash.
if(NOT verilog_content MATCHES "quote=\\\\\" backslash=\\\\\\\\ done")
    message(FATAL_ERROR
        "Regenerated Verilog did not reproduce the escaped string literal byte for byte.\n"
        "Full content:\n${verilog_content}")
endif()

# --- Cone inlining is untouched. --------------------------------------------
# The cone body must still be expanded at its call site with blocking '=',
# which is the invariant hif-backend#16 rests on.
if(NOT verilog_content MATCHES "axb = a \\^ b;")
    message(FATAL_ERROR
        "The inlined cone body is missing from the regenerated Verilog: the system task fix must not disturb "
        "visitProcedureCall's cone-inlining path (hif-backend#16).\nFull content:\n${verilog_content}")
endif()

# --- Reparse. ---------------------------------------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o system_task_calls_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- The property all of the above is really about: it still prints. --------
simulate(original ${FIXTURE} original_trace)
simulate(regenerated ${OUTPUT_VERILOG} regenerated_trace)

if(original_trace STREQUAL "")
    message(FATAL_ERROR "The fixture itself printed nothing, so the comparison would be vacuous.")
endif()

if(NOT original_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated Verilog does not reproduce the source's printed output (hif-backend#29).\n"
        "--- original trace ---\n${original_trace}\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

message(STATUS "system_task_calls test passed.")
