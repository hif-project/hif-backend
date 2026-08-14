# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#38): visitProcedureCall treated every
#           Procedure carrying a StateTable as a frontend-synthesized logic
#           cone and expanded its body at the call site. A user-written Verilog
#           task is also a Procedure with a StateTable, so it took that path -
#           and because a task assigns to the signals and ports the source told
#           it to rather than to the "_sig_var" Variable a cone uses, the
#           hif-backend#16 guard then fired. The tool exited 1 and left a
#           zero-byte output file, reporting a cone invariant about a design
#           that contains no cone.
#
#           A design containing an ordinary Verilog task could therefore not be
#           regenerated at all.
#
#           The fix is a distinction, not a change to either path: a cone is
#           named from hif-frontend's reserved "hif_cone_" stem, and anything
#           else is a task, emitted as `task`/`endtask` and *called* rather than
#           expanded. So this test requires both to be right at once - the task
#           survives as a declaration with a call, and the gate primitive's cone
#           is still inlined into its caller.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o user_task ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/user_task.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

file(READ ${HIF_FILE} hif_content)

# The premise: the frontend records the task as an ordinary Procedure keeping
# the name the source gave it, and synthesizes a cone under its reserved name
# for the gate. Both have to be present or the distinction is untested.
if(NOT hif_content MATCHES "PROCEDURE kind=\"INSTANCE\" name=\"doit\"")
    message(FATAL_ERROR
        "verilog2hif did not record the user task as a Procedure, so this test is not exercising the emission "
        "gap it is about. The frontend, not hif2verilog, would be at fault.\n")
endif()
if(NOT hif_content MATCHES "PCALL name=\"hif_cone_")
    message(FATAL_ERROR
        "verilog2hif synthesized no cone procedure for the gate primitive, so this test cannot show that a task "
        "and a cone are told apart.\n")
endif()

# --- Backend. ----------------------------------------------------------------
# Before the fix this exits 1 on the hif-backend#16 assert.
execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
    OUTPUT_VARIABLE backend_output
    ERROR_VARIABLE backend_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "hif2verilog failed with exit code ${result} (expected 0). A user-written task must not be inlined as a "
        "cone (hif-backend#38):\n${backend_output}")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/user_task.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

# The abort left the file behind at zero bytes, which is the shape #23 and #27
# were about. Checked explicitly rather than inferred from the exit code.
file(SIZE ${OUTPUT_VERILOG} output_size)
if(output_size EQUAL 0)
    message(FATAL_ERROR "Regenerated Verilog is a zero-byte file (hif-backend#38).")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The task survives as a task. --------------------------------------------
if(NOT verilog_content MATCHES "task doit;")
    message(FATAL_ERROR
        "The user-defined task was not emitted as a `task` declaration (hif-backend#38).\n"
        "Full content:\n${verilog_content}")
endif()
if(NOT verilog_content MATCHES "endtask")
    message(FATAL_ERROR
        "The emitted task was not terminated with `endtask` (hif-backend#38).\n"
        "Full content:\n${verilog_content}")
endif()
# Declared once. printList visits declarations too, and it used to visit
# procedures directly - harmless while visitProcedure printed nothing, but it
# declared the task a second time once it did.
string(REGEX MATCHALL "task doit;" doit_declarations "${verilog_content}")
list(LENGTH doit_declarations doit_count)
if(NOT doit_count EQUAL 1)
    message(FATAL_ERROR
        "Expected exactly one declaration of task `doit`, found ${doit_count} (hif-backend#38).\n"
        "Full content:\n${verilog_content}")
endif()

# --- And is called rather than expanded. -------------------------------------
if(NOT verilog_content MATCHES "doit\\(a\\)")
    message(FATAL_ERROR
        "The task is declared but never called: its body was expanded at the call site instead "
        "(hif-backend#38).\nFull content:\n${verilog_content}")
endif()

# --- While the cone is still expanded. ---------------------------------------
# The gate's cone must be inlined into its caller, not emitted as a declaration
# and a call. This is the half hif-backend#16 rests on, and an over-broad fix
# that stopped inlining every Procedure would fail here.
if(verilog_content MATCHES "task hif_cone_")
    message(FATAL_ERROR
        "A frontend-synthesized cone was emitted as a task. Cones are inlined at their call sites; hoisting one "
        "out converts an intra-process dependency into an inter-process one (hif-backend#16).\n"
        "Full content:\n${verilog_content}")
endif()
if(NOT verilog_content MATCHES "axb = a \\^ b")
    message(FATAL_ERROR
        "The inlined cone body is missing from the regenerated Verilog: telling tasks from cones must not "
        "disturb cone inlining (hif-backend#16).\nFull content:\n${verilog_content}")
endif()

# --- Reparse. ----------------------------------------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o user_task_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- And behaves the same. ---------------------------------------------------
# A task emitted with the wrong assignment kind would compile and read stale
# values instead, which only simulation sees.
simulate(original ${FIXTURE} original_trace)
simulate(regenerated ${OUTPUT_VERILOG} regenerated_trace)

if(original_trace STREQUAL "")
    message(FATAL_ERROR "The fixture itself printed nothing, so the comparison would be vacuous.")
endif()

# Pin one line outright so that two equally broken designs cannot agree with
# each other: with a=1 and b=1, y is ~a = 0, carry is b = 1, and the gate chain
# gives g = (a ^ b) ^ 0 = 0.
if(NOT original_trace MATCHES "t=30 a=1 b=1 y=0 z=1 g=0")
    message(FATAL_ERROR "The fixture did not produce the expected values, so the oracle is wrong:\n${original_trace}")
endif()

if(NOT original_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated Verilog does not reproduce the source's behaviour (hif-backend#38).\n"
        "--- original trace ---\n${original_trace}\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

message(STATUS "user_task test passed.")
