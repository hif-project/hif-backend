# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#92): hif2vhdl aborted on every Verilog
#           reduction operator - exit 134 on the Log.cpp assertion, "This
#           operator should be managed in refinement steps", with no VHDL
#           written at all. VHDL before 2008 has no reduction operator, so the
#           three of them need a lowering rather than a token, and the printer
#           refused precisely because the lowering did not exist.
#
#           The lowering builds an explicit chain over the operand's bits, so
#           the test checks three separate things, each of which can fail on
#           its own:
#
#             1. hif2vhdl produces VHDL at all - the reported symptom;
#             2. the emitted VHDL is readable by vhdl2hif - it must not need
#                VHDL-2008, which is why the unary `or a` form was rejected;
#             3. the design still computes what it computed before, checked by
#                routing it the whole way back to Verilog and simulating
#                against Verilog's own reduction operators.
#
#           (3) is what makes this more than a smoke test: a chain built with
#           the wrong bit operator, or over the wrong bits, still produces
#           valid VHDL that reparses.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE VHDL2HIF_EXECUTABLE HIF2VHDL_EXECUTABLE HIF2VERILOG_EXECUTABLE
                 IVERILOG_EXECUTABLE VVP_EXECUTABLE FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o vhdl_reduction_operators ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/vhdl_reduction_operators.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The reduction operators have to still be in the HIF for this to be a lowering
# test at all. If verilog2hif ever started lowering them itself, hif2vhdl would
# never see one and every check below would pass vacuously.
file(READ ${HIF_FILE} hif_content)
foreach(reduction "ORRD" "ANDRD" "XORRD")
    if(NOT hif_content MATCHES "operator=\"${reduction}\"")
        message(FATAL_ERROR
            "verilog2hif produced no ${reduction} operator, so this test is not exercising the lowering it "
            "is about (hif-backend#92).")
    endif()
endforeach()

# --- 1. The reported symptom: hif2vhdl aborted here. -------------------------
execute_process(
    COMMAND ${HIF2VHDL_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/vhdl_out
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "hif2vhdl failed with exit code ${result} (expected 0) -- this is the reported symptom of "
        "hif-backend#92:\n${tool_output}")
endif()

set(OUTPUT_VHDL ${WORK_DIR}/vhdl_out/src/vhdl_reduction_operators.vhd)
if(NOT EXISTS ${OUTPUT_VHDL})
    message(FATAL_ERROR "Expected VHDL not produced: ${OUTPUT_VHDL}")
endif()

file(READ ${OUTPUT_VHDL} vhdl_content)

# The HIF operator names must not survive into the output. VHDLPrinter prints
# op_sla/op_sra as "sla"/"sra", so a reduction leaking through as "orrd" is a
# plausible failure rather than an impossible one.
foreach(leaked "orrd" "andrd" "xorrd")
    if(vhdl_content MATCHES "${leaked}")
        message(FATAL_ERROR
            "The emitted VHDL contains the HIF operator name '${leaked}'.\nFull content:\n${vhdl_content}")
    endif()
endforeach()

# --- 2. The output must be readable back, i.e. must not need VHDL-2008. ------
execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o vhdl_reduction_operators_reparsed ${OUTPUT_VHDL}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "The emitted VHDL failed to reparse (exit code ${result}). The lowering has to stay within the "
        "VHDL vhdl2hif reads.\n${tool_output}\nFull content:\n${vhdl_content}")
endif()

# --- 3. And it must still compute the same thing. ----------------------------
# Routed the whole way back to Verilog, because there is no VHDL simulator here
# and Verilog is where the reference operators live.
execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${WORK_DIR}/vhdl_reduction_operators_reparsed.hif.xml
            -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed on the reparsed design with exit code ${result}:\n${tool_output}")
endif()

set(ROUND_TRIPPED ${WORK_DIR}/verilog_out/vhdl_reduction_operators.v)
if(NOT EXISTS ${ROUND_TRIPPED})
    message(FATAL_ERROR "Expected round-tripped Verilog not produced: ${ROUND_TRIPPED}")
endif()

execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/vhdl_reduction_operators.vvp
            ${ROUND_TRIPPED} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to compile the round-tripped design, exit code ${result}:\n${compile_output}")
endif()

execute_process(
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/vhdl_reduction_operators.vvp
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
        "The design no longer computes Verilog's reduction after the trip through VHDL -- the lowered "
        "chain is not equivalent to the operator it replaced.\n--- trace ---\n${trace}\n"
        "--- emitted VHDL ---\n${vhdl_content}")
endif()

if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

message(STATUS "vhdl_reduction_operators test passed.")
