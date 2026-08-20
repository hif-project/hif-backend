# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#45): a VHDL wait that sets more than one of
#           condition, sensitivity and timeout had no Verilog emission at all.
#           VHDL's three wait clauses are independently optional, so
#           `wait on a until b = '1' for 10 ns;` produces one HIF Wait with all
#           three fields set, and Verilog has no single statement meaning all
#           three. The printer refused it outright (and, before hif-backend#42,
#           spliced the fragments into the following statement).
#
#           It is now lowered into a named block whose concurrent branches each
#           `disable` it. What has to be tested is the *time* each wait resumes
#           at, not that it resumes: a lowering that got the structure right and
#           the resumption logic wrong would still eventually drive every output
#           high. So the fixture arranges for each of the four combinations to
#           resume for a different reason, and the testbench checks the times.
#
#           Two of those reasons cannot be produced by a single sequential
#           block, which is why the shape originally proposed in the issue does
#           not work: with the sensitivity list idle it sits in the event
#           control and never reaches the deadline, so the wait never resumes at
#           all. The fixture ties `a` off precisely to exercise that.
#
#           The reparse leg is on a second fixture, and deliberately. Three of
#           the four combinations need a timeout branch and therefore a `fork`,
#           which verilog2hif cannot parse (hif-frontend#26); the fourth needs
#           none and does reparse. Splitting it that way keeps the round-trip
#           property required of this path wherever it can be, and localises the
#           gap to `fork` rather than letting it excuse the whole test.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE VERILOG2HIF_EXECUTABLE IVERILOG_EXECUTABLE
                 VVP_EXECUTABLE FIXTURE NO_TIMEOUT_FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

function(translate label fixture out_verilog)
    execute_process(
        COMMAND ${VHDL2HIF_EXECUTABLE} -o ${label} ${fixture}
        WORKING_DIRECTORY ${WORK_DIR}
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Setup step (vhdl2hif) failed on ${fixture} with exit code ${result} -- this fixture is "
            "expected to parse cleanly.")
    endif()

    set(hif_file ${WORK_DIR}/${label}.hif.xml)
    if(NOT EXISTS ${hif_file})
        message(FATAL_ERROR "Expected HIF file not produced: ${hif_file}")
    endif()

    execute_process(
        COMMAND ${HIF2VERILOG_EXECUTABLE} ${hif_file} -D ${WORK_DIR}/${label}_out
        RESULT_VARIABLE result
        OUTPUT_VARIABLE tool_output
        ERROR_VARIABLE tool_output
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "hif2verilog failed on ${label} with exit code ${result} -- a multi-clause wait is expected "
            "to be lowered, not refused (hif-backend#45).\n${tool_output}")
    endif()

    set(produced ${WORK_DIR}/${label}_out/${label}.v)
    if(NOT EXISTS ${produced})
        message(FATAL_ERROR "Expected regenerated Verilog not produced: ${produced}")
    endif()
    set(${out_verilog} "${produced}" PARENT_SCOPE)
endfunction()

# --------------------------------------------------------------------------
# The four multi-clause combinations.
# --------------------------------------------------------------------------
translate(multi_clause_wait ${FIXTURE} OUTPUT_VERILOG)
file(READ ${OUTPUT_VERILOG} verilog_content)

# The HIF has to actually carry the multi-clause waits, or the printer has
# nothing to get wrong and everything below passes vacuously.
file(READ ${WORK_DIR}/multi_clause_wait.hif.xml hif_content)
string(REGEX MATCHALL "<WAIT" wait_nodes "${hif_content}")
list(LENGTH wait_nodes wait_count)
if(wait_count LESS 4)
    message(FATAL_ERROR
        "The HIF holds ${wait_count} Wait nodes, expected at least 4 -- this test is not exercising the "
        "combinations it is about (hif-backend#45).")
endif()

# --- Nothing may be refused any more. ----------------------------------------
# The old behaviour was a diagnostic naming this issue; if it comes back, the
# translate() above would already have failed, but a printer that emitted the
# text without stopping would not be caught by exit code alone.
if(verilog_content MATCHES "Unsupported Wait combination")
    message(FATAL_ERROR
        "The refusal text is present in the regenerated Verilog (hif-backend#45).\n"
        "Full content:\n${verilog_content}")
endif()

# --- The lowering's shape. ---------------------------------------------------
# Three of the four need a concurrent timeout branch; all four need a named
# block to leave. A `disable` with no label does not compile, and a label with
# no `disable` exits nothing, so both halves are required.
if(NOT verilog_content MATCHES "begin[ \t]*:[ \t]*hif_wait")
    message(FATAL_ERROR
        "No named wait block in the regenerated Verilog, so there is nothing for a resumption to "
        "disable (hif-backend#45).\nFull content:\n${verilog_content}")
endif()
if(NOT verilog_content MATCHES "disable[ \t]+hif_wait")
    message(FATAL_ERROR
        "No `disable` of a wait block in the regenerated Verilog (hif-backend#45).\n"
        "Full content:\n${verilog_content}")
endif()
string(REGEX MATCHALL "fork" fork_blocks "${verilog_content}")
list(LENGTH fork_blocks fork_count)
if(NOT fork_count EQUAL 3)
    message(FATAL_ERROR
        "Expected exactly 3 `fork` blocks - one per combination carrying a timeout - and found "
        "${fork_count}. A timeout has to run concurrently with the event control; a sequential block "
        "would sit in the event control and never reach the deadline.\n"
        "Full content:\n${verilog_content}")
endif()

# --- The property all of the above is really about: when each wait resumes. --
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/multi_clause_wait.vvp
            ${OUTPUT_VERILOG} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to compile the regenerated design, exit code ${result}:\n${compile_output}\n"
        "Full content:\n${verilog_content}")
endif()

execute_process(
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/multi_clause_wait.vvp
    RESULT_VARIABLE result
    OUTPUT_VARIABLE trace
    ERROR_VARIABLE run_errors
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "vvp failed to run, exit code ${result}:\n${run_errors}")
endif()

string(REPLACE "\r\n" "\n" trace "${trace}")

if(trace MATCHES "FAIL")
    message(FATAL_ERROR
        "A wait resumed at the wrong time, so the lowering does not mean what the VHDL meant "
        "(hif-backend#45).\n--- trace ---\n${trace}\n--- regenerated source ---\n${verilog_content}")
endif()

if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

# --------------------------------------------------------------------------
# The fork-free combination, which must still survive the round trip.
# --------------------------------------------------------------------------
translate(multi_clause_wait_no_timeout ${NO_TIMEOUT_FIXTURE} NO_TIMEOUT_VERILOG)
file(READ ${NO_TIMEOUT_VERILOG} no_timeout_content)

# No timeout means no second reason to resume, so no concurrency is needed.
# Emitting a fork here anyway would put this shape out of the round trip too,
# for nothing.
if(no_timeout_content MATCHES "fork")
    message(FATAL_ERROR
        "`wait on a until b` was lowered with a `fork` although it has no timeout. That has only one "
        "reason to resume, needs no concurrency, and a fork puts it outside what verilog2hif can read "
        "back (hif-frontend#26).\nFull content:\n${no_timeout_content}")
endif()

execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/multi_clause_wait_no_timeout.vvp ${NO_TIMEOUT_VERILOG}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to compile the fork-free lowering, exit code ${result}:\n${compile_output}\n"
        "Full content:\n${no_timeout_content}")
endif()

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o multi_clause_wait_no_timeout_reparsed ${NO_TIMEOUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "The fork-free lowering failed to reparse (exit code ${result}). This is the one combination on "
        "this path that is inside the round trip, and it has to stay there.\n"
        "Full content:\n${no_timeout_content}")
endif()

message(STATUS "multi_clause_wait test passed.")
