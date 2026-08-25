# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#84): `casez` and `casex` were both
#           regenerated as a plain `case`. Under `case` the comparison is exact,
#           so a label like 4'bzzz1 matches only a selector that is literally
#           zzz1 - every wildcard alternative becomes unreachable and control
#           falls through to the default.
#
#           The output is valid Verilog, exits 0 and reparses cleanly; only the
#           values change. So simulation is the test, and the text checks exist
#           so that a failure points at the cause rather than only at a
#           mismatch.
#
#           The HIF carried the distinction all along - Switch::caseSemantics is
#           CASE_Z / CASE_X / CASE_LITERAL - and visitSwitch printed "case"
#           regardless. The plain `case` in the fixture is the control for
#           CASE_LITERAL, which must keep printing "case".
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

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o casez_wildcards ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/casez_wildcards.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The distinction has to be in the HIF for this to be an emission test at all.
# If verilog2hif ever stopped recording it, the printer would have nothing to
# read and the simulation below would be checking the frontend rather than
# this fix.
file(READ ${HIF_FILE} hif_content)
foreach(semantics CASE_Z CASE_X CASE_LITERAL)
    if(NOT hif_content MATCHES "caseSemantics=\"${semantics}\"")
        message(FATAL_ERROR
            "verilog2hif did not record ${semantics} in the HIF, so this test is not exercising the "
            "emission gap it is about (hif-backend#84).")
    endif()
endforeach()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/casez_wildcards.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- One of each keyword, so a fix that reached for the wildcard form ---------
# --- unconditionally is caught as well as one that never reaches for it. ------
foreach(keyword casez casex)
    if(NOT verilog_content MATCHES "${keyword} \\(")
        message(FATAL_ERROR
            "Regenerated Verilog has no `${keyword} (` - the wildcard keyword was not emitted, so every "
            "wildcard alternative is unreachable (hif-backend#84).\nFull content:\n${verilog_content}")
    endif()
endforeach()

# The literal case. Anchored on the leading newline+indent so it cannot be
# satisfied by the "case" inside "casez" or "casex".
if(NOT verilog_content MATCHES "\n[ \t]*case \\(")
    message(FATAL_ERROR
        "Regenerated Verilog has no plain `case (` - CASE_LITERAL must keep printing `case`.\n"
        "Full content:\n${verilog_content}")
endif()

# --- The defect is a value defect, so simulation is what settles it. ---------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/casez_wildcards.vvp ${OUTPUT_VERILOG} ${TESTBENCH}
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
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/casez_wildcards.vvp
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
        "The regenerated design computes the wrong values -- a wildcard alternative was unreachable and "
        "control fell through to the default, which is exactly hif-backend#84.\n"
        "--- trace ---\n${trace}\n--- regenerated source ---\n${verilog_content}")
endif()

if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

message(STATUS "casez_wildcards test passed.")
