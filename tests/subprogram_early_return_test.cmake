# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#63, #73): a subprogram's early `return` was
#           not an exit. visitReturn emitted a function's *value* - the
#           assignment to its own name that #57 added - and nothing at all for
#           the exit, so the statements the `return` was written to skip ran
#           anyway.
#
#           Two observables, one missing lowering:
#
#             #63  a void procedure. The guarded branch is followed by an
#                  unconditional assignment, so the guard has no effect. The
#                  printer at least warned about this one.
#             #73  a function. The trailing `return` overwrites the guarded
#                  one, so the function collapses to a constant. Silent: the
#                  value-carrying branch of visitReturn emitted no warning.
#
#           Simulation is the test. Both defects produce output that compiles,
#           reparses and exits 0; the only symptom is the value, and only for
#           the input the guard was written for. The text checks below exist so
#           that a failure points at the cause rather than only at a mismatch.
#
#           The third subprogram is the control. A single trailing `return`
#           needs no exit - falling off the end of the body already leaves it -
#           so its body must acquire neither a label nor a `disable`. Without
#           that check the fix would be free to wrap every subprogram this
#           printer emits in a named block, changing output that was correct.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE VERILOG2HIF_EXECUTABLE IVERILOG_EXECUTABLE
                 VVP_EXECUTABLE FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o subprogram_early_return ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/subprogram_early_return.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# Four Returns have to be in the HIF - one in the procedure, two in `pick`, one
# in `ident`. If vhdl2hif ever stopped producing them, or started flattening the
# guards itself, the printer would have nothing to get wrong and every check
# below would pass vacuously.
file(READ ${HIF_FILE} hif_content)
string(REGEX MATCHALL "<RETURN" return_nodes "${hif_content}")
list(LENGTH return_nodes return_count)
if(return_count LESS 4)
    message(FATAL_ERROR
        "The HIF holds ${return_count} Return nodes, expected at least 4, so this test is not exercising "
        "the emission gap it is about (hif-backend#63, #73).")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/subprogram_early_return.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The lowering is present, in both subprogram kinds. ----------------------
# `return` in a task is SystemVerilog and the suite builds with -g2005, so the
# only available spelling is a named block plus `disable`. Both halves are
# required: a label with no disable exits nothing, a disable with no label does
# not compile.
foreach(subprogram maybe pick)
    if(NOT verilog_content MATCHES "begin[ \t]*:[ \t]*${subprogram}_body")
        message(FATAL_ERROR
            "The body of '${subprogram}' is not a named block, so there is nothing for an early return to "
            "disable (hif-backend#63, #73).\nFull content:\n${verilog_content}")
    endif()
    if(NOT verilog_content MATCHES "disable[ \t]+${subprogram}_body")
        message(FATAL_ERROR
            "'${subprogram}' has no `disable` for its early return, so the statements after it still run "
            "(hif-backend#63, #73).\nFull content:\n${verilog_content}")
    endif()
endforeach()

# --- And absent where it is not needed. --------------------------------------
if(verilog_content MATCHES "ident_body")
    message(FATAL_ERROR
        "'ident' has a single trailing return and needs no exit, but its body was wrapped in a named "
        "block anyway. The lowering must not change subprograms that were already correct.\n"
        "Full content:\n${verilog_content}")
endif()

# --- `return` itself must never be emitted: it is SystemVerilog. -------------
if(verilog_content MATCHES "[ \t\n]return[ \t;]")
    message(FATAL_ERROR
        "The regenerated Verilog contains a `return` statement, which is SystemVerilog and not accepted "
        "by iverilog -g2005.\nFull content:\n${verilog_content}")
endif()

# --- Reparse, since the frontend has to accept what the backend wrote. -------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o subprogram_early_return_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- The property all of the above is really about. --------------------------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/subprogram_early_return.vvp
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
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/subprogram_early_return.vvp
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
        "The regenerated design computes the wrong values -- an early return did not exit, so the "
        "statements it guards ran anyway (hif-backend#63, #73).\n--- trace ---\n${trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

message(STATUS "subprogram_early_return test passed.")
