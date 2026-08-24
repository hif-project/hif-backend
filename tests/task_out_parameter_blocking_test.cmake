# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#70): an assignment to a task's out or inout
#           argument was emitted with the non-blocking operator "<=".
#
#               task maybe;
#                   output s;
#                   input v;
#                   begin
#                       s <= 1'b1;    <-- copy-back has already run by then
#                   end
#               endtask
#
#           VerilogPrinter::visitAssign chose the operator from the target
#           declaration's class - Variable got "=", everything else got "<=" -
#           and a hif::Parameter is not a hif::Variable. But a task argument is
#           copied back to its actual when the task *returns*, while "<=" only
#           updates the local at the end of the time step, i.e. afterwards.
#
#           Needs a simulator, and the timing of each read is the test. The
#           output compiles, reparses and exits 0 both before and after the fix;
#           the only symptom is a value, and for the out case only at the first
#           activation, because the lag drains after that. Reading later would
#           pass against the defect.
#
#           No reparse leg: the fixture's inout procedure regenerates as a task
#           `inout` argument, which is legal Verilog-2001 that iverilog accepts
#           but verilog2hif refuses outright (hif-frontend#25). The out half's
#           reparse is already covered by task_out_parameter.
#
#           Observed against the pre-fix binary, which is what says this test
#           discriminates:
#
#               FAIL: y_out after 1st activation is x, expected 1
#               FAIL: y_inout after 4 activations is x, expected 0
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE VVP_EXECUTABLE
                 FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o task_out_parameter_blocking ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/task_out_parameter_blocking.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# Both directions have to be present, or the test silently stops covering half
# of what it is about.
file(READ ${HIF_FILE} hif_content)
foreach(direction "OUT" "INOUT")
    if(NOT hif_content MATCHES "<PARAMETER[^>]*direction=\"${direction}\"")
        message(FATAL_ERROR
            "vhdl2hif produced no dir_${direction} Parameter, so this test is not exercising the case "
            "it is about (hif-backend#70).")
    endif()
endforeach()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/task_out_parameter_blocking.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The property all of the above is really about. --------------------------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/task_out_parameter_blocking.vvp
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
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/task_out_parameter_blocking.vvp
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
        "A task argument's assignment is not visible to the copy-back that follows it, so the design "
        "publishes a stale value (hif-backend#70).\n--- trace ---\n${trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

message(STATUS "task_out_parameter_blocking test passed.")
