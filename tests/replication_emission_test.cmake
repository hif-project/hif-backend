# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#61): a one-bit value was emitted as a bare
#           digit, so `{1'b1, 1'b0}` regenerated as `{1, 0}`. Verilog gives an
#           unsized `1` a self-determined 32-bit width and a concatenation
#           rejects that outright:
#
#             error: Concatenation operand "'sd1" has indefinite width.
#
#           A non-0/1 bit was worse: it rendered as the empty string, giving
#           `{{{, }, 1'bz}, 1'bz}` and a syntax error. Exit code 0 in both cases,
#           so nothing reported it until the next tool read the output.
#
#           Filed against replication, but replication is only one way to build
#           a concatenation. `pc` in the fixture has no replication and failed
#           identically, which is why the assertions below are about
#           concatenation operands rather than about replication.
#
#           Why position appeared to matter: VerilogParser::concat builds a
#           left-leaning nest of identical copies, and a later type unification
#           promotes an operand to a one-bit Bitvector once its sibling is
#           already vector-typed. Only the innermost pair stays a Bit, and the
#           two kinds rendered differently. After the fix both render as sized
#           literals, so nothing depends on position.
#
#           The simulation comparison is the real gate; the text checks make a
#           failure name the cause instead of only a mismatch.
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
    set(image ${WORK_DIR}/${label}.vvp)
    execute_process(
        COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${image} ${source} ${TESTBENCH}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE compile_output
        ERROR_VARIABLE compile_output
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "iverilog failed to build ${label} (${source}), exit ${result}:\n${compile_output}")
    endif()
    execute_process(
        COMMAND ${VVP_EXECUTABLE} ${image}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE trace
        ERROR_VARIABLE run_errors
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "vvp failed to run ${label}, exit ${result}:\n${run_errors}")
    endif()
    string(REPLACE "\r\n" "\n" trace "${trace}")
    string(REGEX REPLACE "\n[^\n]*\\$finish[^\n]*" "" trace "${trace}")
    string(STRIP "${trace}" trace)
    set(${out_trace} "${trace}" PARENT_SCOPE)
endfunction()

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o replication_emission ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit ${result} -- this fixture is expected to translate cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/replication_emission.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The concatenations must still be concatenations for this to test emission. If
# a future frontend or simplify change folds them all into sized literals, the
# printer path under test is never reached and everything below would pass while
# covering nothing. That is exactly what a too-wide target does, and it is why
# the fixture declares every output at its natural width.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "operator=\"CONCAT\"")
    message(FATAL_ERROR
        "The HIF contains no CONCAT expression, so the concatenation-operand emission this test is about "
        "is never exercised (hif-backend#61).")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/replication_emission.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- No empty concatenation operand. -----------------------------------------
# This is what a non-0/1 bit used to produce: `{{{, }, 1'bz}, 1'bz}`.
if(verilog_content MATCHES "\\{[ \t]*,|,[ \t]*\\}")
    message(FATAL_ERROR
        "Regenerated Verilog contains an empty concatenation operand; a four-state bit rendered as nothing "
        "(hif-backend#61).\nFull content:\n${verilog_content}")
endif()

# --- No unsized concatenation operand. ---------------------------------------
# Matches a run of digits sitting alone between concatenation delimiters, e.g.
# `{1, 0}` or `, 1}`. A sized literal such as 4'b1010 or 2'b01 does not match,
# because its digits are followed by an apostrophe rather than by , or }.
if(verilog_content MATCHES "[{,][ \t]*[0-9]+[ \t]*[,}]")
    message(FATAL_ERROR
        "Regenerated Verilog contains an unsized concatenation operand; Verilog gives it a 32-bit "
        "self-determined width and elaboration fails with \"indefinite width\" (hif-backend#61).\n"
        "Full content:\n${verilog_content}")
endif()

# --- The four-state operands survive as themselves. --------------------------
foreach(literal "1'bx" "1'bz")
    if(NOT verilog_content MATCHES "${literal}")
        message(FATAL_ERROR
            "Regenerated Verilog lost the ${literal} operand; a four-state bit must be emitted, not dropped "
            "(hif-backend#61).\nFull content:\n${verilog_content}")
    endif()
endforeach()

# --- The symbolic replication count is preserved, not elaborated. ------------
# {N{1'b1}} must come back as a replication over the parameter rather than as a
# folded constant: the toolchain's existing contract keeps a non-constant count
# symbolic, and a fix that elaborated it would silently change that.
if(NOT verilog_content MATCHES "\\{N\\{")
    message(FATAL_ERROR
        "The parameterized replication {N{1'b1}} was not preserved as a replication over N.\n"
        "Full content:\n${verilog_content}")
endif()

# --- Reparse. ----------------------------------------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o replication_emission_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit ${result}).\nFull content:\n${verilog_content}")
endif()

# --- The property all of the above is really about. --------------------------
# Exhaustive over the 16 values of `in`, and four-state aware.
simulate(original ${FIXTURE} original_trace)
simulate(regenerated ${OUTPUT_VERILOG} regenerated_trace)

if(NOT original_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated design is not behaviourally equivalent to the source (hif-backend#61).\n"
        "--- original ---\n${original_trace}\n"
        "--- regenerated ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

# The comparison must not be vacuous: an input-dependent output has to actually
# vary across the sweep, otherwise two equally-broken designs would compare equal.
if(NOT original_trace MATCHES "rvec=0101")
    message(FATAL_ERROR
        "The reference trace does not show the input-dependent replication varying, so the equivalence "
        "comparison proves nothing:\n${original_trace}")
endif()

message(STATUS "replication_emission test passed.")
