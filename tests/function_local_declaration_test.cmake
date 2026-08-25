# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#83): a function-local variable was emitted
#           as `reg [4:0] wide_fn = 5'bxxxxx;` inside the function body.
#           Verilog allows a variable declaration assignment only at module
#           level, so the regenerated file did not parse - neither iverilog nor
#           verilog2hif would take it, while hif2verilog itself exited 0.
#
#           The all-x value is verilog2hif's default for a reg the source left
#           uninitialized. It says nothing the declaration does not already say,
#           so the correct output here is a bare declaration: the value is
#           neither restated on the declaration nor re-emitted as a statement.
#           The VHDL side, where the value IS something the source wrote, is
#           covered by subprogram_local_initialization.
#
#           The task in the same fixture is a control. verilog2hif hoists a
#           Verilog task's local to module level, where a declaration assignment
#           is legal, so that shape was never broken and must stay untouched.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o function_local_declaration ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/function_local_declaration.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The local has to still be inside the function in the HIF for this to be an
# emission test at all. If verilog2hif ever hoists a function's local the way it
# hoists a task's, the printer would never see this shape and every check below
# would pass vacuously.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "wide_fn")
    message(FATAL_ERROR
        "verilog2hif produced no 'wide_fn' declaration, so this test is not exercising the emission gap "
        "it is about (hif-backend#83).")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/function_local_declaration.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The function-local is declared, and declared bare. ----------------------
if(NOT verilog_content MATCHES "reg \\[4:0\\] wide_fn;")
    message(FATAL_ERROR
        "The function-local 'wide_fn' is not declared as a bare `reg [4:0] wide_fn;`.\n"
        "Full content:\n${verilog_content}")
endif()

# The exact broken form. Matching the value as well as the name means a body
# assignment of the frontend default would be caught too, not just a declaration
# assignment.
if(verilog_content MATCHES "wide_fn = 5'bxxxxx")
    message(FATAL_ERROR
        "The frontend's all-x default was re-emitted for 'wide_fn' - as a declaration assignment, which "
        "Verilog allows only at module level, or as a statement, which says nothing an uninitialized reg "
        "does not already say (hif-backend#83).\nFull content:\n${verilog_content}")
endif()

# --- Control: the task's local was hoisted by the frontend and keeps its ------
# --- module-level declaration assignment, which is legal there. ---------------
if(NOT verilog_content MATCHES "reg \\[4:0\\] wide_tk = 5'bxxxxx;")
    message(FATAL_ERROR
        "The task-local 'wide_tk', hoisted to module level by verilog2hif, lost its declaration "
        "assignment. That shape was never broken and must stay untouched.\n"
        "Full content:\n${verilog_content}")
endif()

# --- The reported symptom: the output has to build. --------------------------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/function_local_declaration.vvp ${OUTPUT_VERILOG}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to compile the regenerated design, exit code ${result} -- this is the reported "
        "symptom of hif-backend#83:\n${compile_output}\nFull content:\n${verilog_content}")
endif()

# --- And it has to survive the trip back in. ---------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o function_local_declaration_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Regenerated Verilog failed to reparse (exit code ${result}).")
endif()

message(STATUS "function_local_declaration test passed.")
