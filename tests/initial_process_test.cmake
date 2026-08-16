# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#40): visitStateTable wrote `always` for
#           every non-analog process and appended `@( ... )` only when one of
#           the three sensitivity lists was non-empty. A process with no
#           sensitivity list therefore came back as a bare `always begin ... end`
#           - a zero-delay infinite loop, which Icarus rejects at elaboration.
#           Every design containing an `initial` block regenerated into
#           something that does not compile, and hif2verilog exited 0 saying so
#           nowhere.
#
#           The rule the fix rests on is re-triggerability: a process is emitted
#           as `always` only if something can wake it up again, meaning it has a
#           sensitivity list or it suspends on a `wait`. A process with neither
#           runs exactly once, which Verilog spells `initial`.
#
#           This test therefore pins both directions. Getting it wrong the other
#           way - demoting a process that does re-trigger - compiles cleanly and
#           silently stops the design responding, so initial_process_wait.v
#           guards that half.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE VVP_EXECUTABLE
                 FIXTURE WAIT_FIXTURE TESTBENCH WORK_DIR)
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

# --- Frontend. ---------------------------------------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o initial_process ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/initial_process.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The frontend has to have recorded the initial blocks as processes carrying
# the INITIAL flavour, or this test is about the frontend rather than about
# hif2verilog's choice of keyword.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "flavour=\"INITIAL\"")
    message(FATAL_ERROR
        "verilog2hif recorded no process with the INITIAL flavour, so this test is not exercising the emission "
        "gap it is about. The frontend, not hif2verilog, would be at fault.\n")
endif()

# --- Backend. ----------------------------------------------------------------
execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/initial_process.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The run-once processes came back as `initial`. --------------------------
if(NOT verilog_content MATCHES "initial begin")
    message(FATAL_ERROR
        "Regenerated Verilog contains no `initial` block: a process with nothing to wake it up was emitted as "
        "`always`, which is a zero-delay infinite loop (hif-backend#40).\n"
        "Full content:\n${verilog_content}")
endif()

# And nothing was left as a delay-free `always`, which is the defect itself:
# every emitted `always` has to carry a sensitivity list. (The frontend merges
# the fixture's two initial blocks into one process, so the count of `initial`
# keywords is not the thing to assert on - the absence of a bare `always` is.)
if(verilog_content MATCHES "always begin")
    message(FATAL_ERROR
        "Regenerated Verilog still contains a bare `always begin`, which is a zero-delay infinite loop "
        "(hif-backend#40).\nFull content:\n${verilog_content}")
endif()

# --- The re-triggerable process was left alone. ------------------------------
# An over-broad fix that keyed on "empty sensitivity list" alone would still
# pass every check above while breaking this one.
if(NOT verilog_content MATCHES "always @\\(")
    message(FATAL_ERROR
        "The sensitive process lost its `always @( ... )`: the fix must change only processes that cannot be "
        "re-triggered (hif-backend#40).\nFull content:\n${verilog_content}")
endif()

# --- It compiles. ------------------------------------------------------------
# This is the check that fails before the fix: the output was syntactically
# fine but elaboration rejected the delay-free always loop.
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/elaborate.vvp ${OUTPUT_VERILOG}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog does not elaborate (hif-backend#40), exit code ${result}:\n${compile_output}\n"
        "Full content:\n${verilog_content}")
endif()

# --- Reparse. ----------------------------------------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o initial_process_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- And it behaves the same. ------------------------------------------------
# Compilation alone would not notice an initial block that was emitted with its
# statements reordered, or one that lost the dependency between them.
simulate(original ${FIXTURE} original_trace)
simulate(regenerated ${OUTPUT_VERILOG} regenerated_trace)

if(original_trace STREQUAL "")
    message(FATAL_ERROR "The fixture itself printed nothing, so the comparison would be vacuous.")
endif()

# Pin the values rather than only comparing the two sides, so that two equally
# broken designs cannot agree with each other.
if(NOT original_trace MATCHES "once=5")
    message(FATAL_ERROR "The fixture did not produce once=5, so the oracle is wrong:\n${original_trace}")
endif()
if(NOT original_trace MATCHES "dep=2")
    message(FATAL_ERROR "The fixture did not produce dep=2, so the oracle is wrong:\n${original_trace}")
endif()

if(NOT original_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated Verilog does not reproduce the source's behaviour (hif-backend#40).\n"
        "--- original trace ---\n${original_trace}\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

# --- The other half of the rule: a `wait` keeps the process an `always`. -----
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o initial_process_wait ${WAIT_FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif on the wait fixture) failed with exit code ${result}.")
endif()

set(WAIT_HIF_FILE ${WORK_DIR}/initial_process_wait.hif.xml)
file(READ ${WAIT_HIF_FILE} wait_hif_content)

# The premise: this process really does have an empty StateTable sensitivity
# list, so it is the case that a fix keyed on emptiness alone would get wrong.
# Its sensitivity lives on the WAIT instead.
if(NOT wait_hif_content MATCHES "<WAIT>")
    message(FATAL_ERROR
        "The wait fixture produced no WAIT node, so it does not exercise the distinction this half of the test "
        "is about.\n")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${WAIT_HIF_FILE} -D ${WORK_DIR}/verilog_out_wait
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed on the wait fixture with exit code ${result} (expected 0)")
endif()

file(READ ${WORK_DIR}/verilog_out_wait/initial_process_wait.v wait_verilog_content)

if(wait_verilog_content MATCHES "initial begin")
    message(FATAL_ERROR
        "A process that suspends on a `wait` was emitted as `initial`. It runs more than once, so this silently "
        "stops the design responding after its first pass (hif-backend#40).\n"
        "Full content:\n${wait_verilog_content}")
endif()
if(NOT wait_verilog_content MATCHES "always")
    message(FATAL_ERROR
        "A process that suspends on a `wait` lost its `always` keyword (hif-backend#40).\n"
        "Full content:\n${wait_verilog_content}")
endif()

message(STATUS "initial_process test passed.")
