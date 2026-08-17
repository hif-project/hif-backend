# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#57): a user-defined function was emitted
#           with an empty body. The `function`/`endfunction` wrapper and the
#           parameters were all present, and the call sites were correct, so the
#           output looked right - but the body was gone, every call returned x,
#           and every condition built on one selected the wrong branch.
#
#           Exit code 0, and the output compiled and simulated. Only running it
#           shows the design no longer computes anything, so simulation is the
#           test; the text checks exist so that a failure points at the cause
#           rather than only at a mismatch.
#
#           Root cause was that visitReturn skipped every Return unconditionally.
#           Verilog has no `return` - a function yields its value by assigning to
#           its own name - so a VHDL function, whose whole body is a Return,
#           lost everything. A verilog2hif-derived function was unaffected: it
#           carries a `<name>_return` variable that its body assigns, and the
#           trailing Return really is redundant there. Both shapes are covered
#           by this suite: this test owns the VHDL one, and the pre-existing
#           system_function_round_trip / user_task tests keep the Verilog one
#           honest.
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
    COMMAND ${VHDL2HIF_EXECUTABLE} -o user_function_body ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/user_function_body.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The body has to be in the HIF for this to be an emission test at all. If
# vhdl2hif ever stops producing a Return here, the printer would have nothing to
# emit and every check below would pass vacuously for the wrong reason.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "<RETURN")
    message(FATAL_ERROR
        "vhdl2hif produced no Return in the function bodies, so this test is not exercising the emission "
        "gap it is about (hif-backend#57).")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/user_function_body.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- Each function assigns its own name, which is how Verilog returns. -------
foreach(function_name both_high either_high invert)
    if(NOT verilog_content MATCHES "${function_name} = ")
        message(FATAL_ERROR
            "Function '${function_name}' has no assignment to its own name, so it returns x: the Return "
            "carrying its value was dropped (hif-backend#57).\nFull content:\n${verilog_content}")
    endif()
endforeach()

# --- No function may be emitted with an empty body. --------------------------
# The literal shape the defect produced, so a regression names itself.
if(verilog_content MATCHES "begin[ \t\r\n]*end[ \t\r\n]*endfunction")
    message(FATAL_ERROR
        "A function is emitted with an empty body (hif-backend#57).\nFull content:\n${verilog_content}")
endif()

# --- The vector-returning function keeps its width. --------------------------
if(NOT verilog_content MATCHES "function[ \t]+\\[3:0\\][ \t]+invert")
    message(FATAL_ERROR
        "The vector-returning function lost its return width, so the assignment truncates "
        "(hif-backend#57).\nFull content:\n${verilog_content}")
endif()

# --- Reparse, since the frontend has to accept what the backend wrote. -------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o user_function_body_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- The property all of the above is really about. --------------------------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/user_function_body.vvp ${OUTPUT_VERILOG} ${TESTBENCH}
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
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/user_function_body.vvp
    RESULT_VARIABLE result
    OUTPUT_VARIABLE trace
    ERROR_VARIABLE run_errors
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "vvp failed to run, exit code ${result}:\n${run_errors}")
endif()

string(REPLACE "\r\n" "\n" trace "${trace}")

# The testbench is self-checking, so a wrong value names itself.
if(trace MATCHES "FAIL")
    message(FATAL_ERROR
        "The regenerated design computes the wrong values -- a function returned x, which is exactly "
        "hif-backend#57.\n--- trace ---\n${trace}\n--- regenerated source ---\n${verilog_content}")
endif()

# And the run has to have actually checked something: a testbench that compiled
# but produced no checks would otherwise pass by saying nothing.
if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

message(STATUS "user_function_body test passed.")
