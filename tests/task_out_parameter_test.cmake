# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#64): a procedure parameter whose direction
#           is not `in` was emitted as a bare `;`. getDeclaration handled only
#           dir_in for a hif::Parameter, so `out` and `inout` produced the empty
#           string, and printTask writes the terminator unconditionally:
#
#               task maybe;
#                   ;
#                   input v;
#
#           The task then referenced a name it never declared, and iverilog
#           rejected the file ("syntax error", "Malformed statement"), while
#           hif2verilog exited 0.
#
#           Text checks alone are not the point here, and neither is exit 0.
#           What the defect really cost was a task that could not be compiled at
#           all, so the gate is: the parameters are declared with the right
#           keyword, the file compiles under iverilog, verilog2hif reads it back,
#           and the design actually publishes the values its procedures assign.
#
#           The value leg used to stop short of the one output that mirrors an
#           input, because hif-backend#70 - a task's out parameter copied back
#           one activation late, the assignment to it being emitted non-blocking
#           - would have made that assertion a test of #70 rather than of this
#           defect. #70 is fixed, so it is asserted now; its own timing-sensitive
#           regression is task_out_parameter_blocking.
#
#           The dir_inout case is covered by task_inout_parameter, separately,
#           because it cannot pass the reparse leg (hif-frontend#25).
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
    COMMAND ${VHDL2HIF_EXECUTABLE} -o task_out_parameter ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/task_out_parameter.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The directions have to be in the HIF for this to be an emission test at all.
# If vhdl2hif ever stopped marking the parameter dir_out, the printer would have
# nothing to get wrong and every check below would pass for the wrong reason.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "<PARAMETER[^>]*direction=\"OUT\"")
    message(FATAL_ERROR
        "vhdl2hif produced no dir_out Parameter, so this test is not exercising the declaration gap it "
        "is about (hif-backend#64).")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/task_out_parameter.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The literal shape of the defect, so a regression names itself. ----------
# A line that is nothing but a semicolon is what an unrendered declaration
# produced. Anchored on the whole line: a `;` is otherwise everywhere.
if(verilog_content MATCHES "\n[ \t]*;[ \t]*\r?\n")
    message(FATAL_ERROR
        "The regenerated Verilog contains an empty declaration - a line holding only ';' - which is a "
        "parameter getDeclaration did not render (hif-backend#64).\nFull content:\n${verilog_content}")
endif()

# --- Each parameter is declared, with the keyword its direction calls for. ----
foreach(expected "output s" "input src" "output dst")
    if(NOT verilog_content MATCHES "${expected}")
        message(FATAL_ERROR
            "The regenerated Verilog does not declare '${expected}', so a task argument is missing or "
            "carries the wrong direction (hif-backend#64).\nFull content:\n${verilog_content}")
    endif()
endforeach()

# --- Reparse: the frontend has to accept what the backend wrote. --------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o task_out_parameter_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- The property all of the above is really about. --------------------------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/task_out_parameter.vvp ${OUTPUT_VERILOG} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to compile the regenerated design, exit code ${result} -- which is the reported "
        "symptom of hif-backend#64:\n${compile_output}\nFull content:\n${verilog_content}")
endif()

execute_process(
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/task_out_parameter.vvp
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
        "The regenerated design does not publish the values its procedures assign, so an out parameter "
        "is not really connected to its actual.\n--- trace ---\n${trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

message(STATUS "task_out_parameter test passed.")
