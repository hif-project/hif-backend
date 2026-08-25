# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#83), VHDL side. A VHDL subprogram's local
#           variable may state an initial value, applied on every call, and it
#           arrived in the regenerated Verilog as a declaration assignment
#           inside the function or task body:
#
#               reg [3:0] m = 4'b0011;   -- inside function ... endfunction
#
#           Verilog allows that only at module level, so the output did not
#           build. Both subprogram kinds are covered here because both reach the
#           same printer path: verilog2hif hoists a Verilog task's local to
#           module level, but a VHDL *procedure* becomes a Verilog task with its
#           local still inside it, so the task path was broken identically.
#
#           Simulation is the test, not compilation. The broken output did not
#           build at all, so a compile-only check would also be satisfied by
#           deleting the initial value - and that is the worse outcome, a design
#           that builds, reparses and computes x for every input. The text
#           checks exist so that a failure points at the cause rather than only
#           at a mismatch.
#
#           The companion function_local_declaration test owns the Verilog side,
#           where the value is verilog2hif's all-x default and the correct
#           output is a bare declaration with nothing re-emitted.
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
    COMMAND ${VHDL2HIF_EXECUTABLE} -o subprogram_local_initialization ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/subprogram_local_initialization.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The initial values have to be on the locals in the HIF for this to be an
# emission test at all. If vhdl2hif ever stopped carrying them, the printer
# would have nothing to place and the simulation below would be checking the
# frontend rather than this fix.
file(READ ${HIF_FILE} hif_content)
foreach(expected_value "0011" "0110")
    if(NOT hif_content MATCHES "${expected_value}")
        message(FATAL_ERROR
            "vhdl2hif did not carry the local's initial value ${expected_value} into the HIF, so this "
            "test is not exercising the emission gap it is about (hif-backend#83).")
    endif()
endforeach()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/subprogram_local_initialization.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- Each local is declared bare, and initialised as a statement. ------------
# Both halves are checked: the declaration must not carry the value (that is the
# construct Verilog rejects), and the value must appear somewhere (dropping it
# would build, and be wrong).
if(NOT verilog_content MATCHES "reg \\[3:0\\] m;")
    message(FATAL_ERROR
        "A subprogram local is not declared bare - the initial value is still on the declaration, which "
        "Verilog allows only at module level (hif-backend#83).\nFull content:\n${verilog_content}")
endif()

foreach(mask "4'b0011" "4'b0110")
    if(NOT verilog_content MATCHES "m = ${mask};")
        message(FATAL_ERROR
            "The local's initial value ${mask} is not assigned anywhere: it was dropped rather than "
            "moved into the body, which builds and computes x for every input.\n"
            "Full content:\n${verilog_content}")
    endif()
endforeach()

# --- The reported symptom: the output has to build. --------------------------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/subprogram_local_initialization.vvp
            ${OUTPUT_VERILOG} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to compile the regenerated design, exit code ${result} -- this is the reported "
        "symptom of hif-backend#83:\n${compile_output}\nFull content:\n${verilog_content}")
endif()

# --- And it has to compute what the VHDL says it computes. -------------------
execute_process(
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/subprogram_local_initialization.vvp
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
        "The regenerated design computes the wrong values -- a subprogram local did not start from its "
        "stated initial value.\n--- trace ---\n${trace}\n--- regenerated source ---\n${verilog_content}")
endif()

if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

message(STATUS "subprogram_local_initialization test passed.")
