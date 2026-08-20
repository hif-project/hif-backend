# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#64), dir_inout half: a procedure parameter
#           declared `signal ... : inout` was emitted as a bare `;`, exactly as
#           an `out` one was, so the task did not parse.
#
#           Split from task_out_parameter because it cannot take the same gate,
#           and the reasons are worth stating rather than leaving as an
#           unexplained weaker test:
#
#           - No reparse leg. The correct emission is a Verilog task `inout`
#             argument, which is legal Verilog-2001 (IEEE Std 1364-2005, 10.2.1)
#             and which iverilog accepts, but `verilog2hif` refuses it outright -
#             hif-frontend#25, filed from exactly this observation.
#
#           - No value assertion. A task `inout` argument is copied *in* at entry
#             as well as out at return, so with the non-blocking assignment
#             hif-backend#70 emits, every call overwrites the local with the
#             still-stale actual before scheduling the new value: the actual
#             never takes it. Measured over four activations, the signal stays
#             `x`. Asserting a value here would be testing #70, not #64.
#
#           What is left is exactly what #64 is about: the parameter is declared,
#           with the `inout` keyword, and the file compiles. Tighten this test in
#           both directions once #70 and hif-frontend#25 are fixed.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o task_inout_parameter ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/task_inout_parameter.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "<PARAMETER[^>]*direction=\"INOUT\"")
    message(FATAL_ERROR
        "vhdl2hif produced no dir_inout Parameter, so this test is not exercising the case it is about "
        "(hif-backend#64).")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/task_inout_parameter.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

if(verilog_content MATCHES "\n[ \t]*;[ \t]*\r?\n")
    message(FATAL_ERROR
        "The regenerated Verilog contains an empty declaration - a line holding only ';' - which is the "
        "unrendered inout parameter (hif-backend#64).\nFull content:\n${verilog_content}")
endif()

if(NOT verilog_content MATCHES "inout s")
    message(FATAL_ERROR
        "The regenerated Verilog does not declare the task argument as 'inout s' (hif-backend#64).\n"
        "Full content:\n${verilog_content}")
endif()

execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/task_inout_parameter.vvp ${OUTPUT_VERILOG}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to compile the regenerated design, exit code ${result} -- which is the reported "
        "symptom of hif-backend#64:\n${compile_output}\nFull content:\n${verilog_content}")
endif()

message(STATUS "task_inout_parameter test passed.")
