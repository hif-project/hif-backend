# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#32): hif2verilog never printed a view's
#           GlobalAction. vhdl2hif puts every VHDL concurrent signal assignment
#           there, so translating VHDL to Verilog produced a module with the
#           correct ports and an empty body - exit code 0, no diagnostic, and
#           output that both compiles and reparses. It simply drove nothing.
#
#           Designs arriving through verilog2hif were unaffected, because that
#           frontend rewrites continuous assignments into processes. That is
#           why no existing test and no corpus design caught this, and why the
#           fixture here has to be VHDL.
#
#           Also covers the wire/reg consequence: whatever a continuous assign
#           drives must be a net, so the hif-backend#26 rule (nets bound to an
#           instance's output port are wires) had to widen to cover continuous
#           assignment targets too.
#
#           And the delay: "after 2 ns" is carried by the same machinery
#           hif-backend#24 added, but no VHDL-derived assignment was ever
#           printed to carry it, so this is the first path on which it runs.
#
#           There is no VHDL simulator here (no ghdl/nvc, locally or in CI), so
#           the source cannot be simulated for comparison. The oracle is
#           computed by hand from the VHDL and stated literally below.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE VERILOG2HIF_EXECUTABLE
                 IVERILOG_EXECUTABLE VVP_EXECUTABLE FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o global_action_emission ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/global_action_emission.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The concurrent assignments must be in the HIF for this test to be about
# emission at all. If they are not, the frontend is at fault, not hif2verilog.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "<GLOBALACTION>")
    message(FATAL_ERROR
        "vhdl2hif did not record the concurrent assignments as a GLOBALACTION, so this test is not exercising "
        "the emission gap it is about. The frontend, not hif2verilog, would be at fault.")
endif()
if(NOT hif_content MATCHES "<DELAY>")
    message(FATAL_ERROR
        "vhdl2hif did not record the \"after 2 ns\" delay, so the delay half of this test is vacuous.")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/global_action_emission.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The body exists at all. This is the bug in its most direct form. --------
if(NOT verilog_content MATCHES "assign ")
    message(FATAL_ERROR
        "Regenerated Verilog contains no continuous assignment: the view's GLOBALACTION was dropped and the "
        "module body is empty (hif-backend#32).\nFull content:\n${verilog_content}")
endif()

# --- Each of the four assignments, named individually so a partial ----------
# --- regression points at which shape broke. --------------------------------
foreach(expected "assign s = a & b" "assign t = s" "assign u = c | s" "assign #2 d = a \\^ b")
    if(NOT verilog_content MATCHES "${expected}")
        message(FATAL_ERROR
            "Regenerated Verilog is missing the continuous assignment '${expected}' (hif-backend#32).\n"
            "Full content:\n${verilog_content}")
    endif()
endforeach()

# --- Whatever a continuous assign drives must be a net (hif-backend#26/#32). -
# The internal signal and all three outputs are driven by an assign, so all
# four are wires. Emitting any of them as reg is Verilog no simulator accepts.
if(NOT verilog_content MATCHES "wire s")
    message(FATAL_ERROR
        "Internal signal 's' is driven by a continuous assign but was not declared as a wire; Verilog forbids "
        "a continuous assign to a reg (hif-backend#26/#32).\nFull content:\n${verilog_content}")
endif()
foreach(port t u d)
    if(NOT verilog_content MATCHES "output wire ${port}")
        message(FATAL_ERROR
            "Output port '${port}' is driven by a continuous assign but was not declared as 'output wire' "
            "(hif-backend#26/#32).\nFull content:\n${verilog_content}")
    endif()
endforeach()
if(verilog_content MATCHES "output reg" OR verilog_content MATCHES "reg s")
    message(FATAL_ERROR
        "Regenerated Verilog still declares a continuously driven target as reg.\n"
        "Full content:\n${verilog_content}")
endif()

# --- The delay counts a declared unit. --------------------------------------
if(NOT verilog_content MATCHES "`timescale ")
    message(FATAL_ERROR
        "Regenerated Verilog carries a '#2' delay but no `timescale, so the delay counts an undefined unit "
        "(hif-backend#24).\nFull content:\n${verilog_content}")
endif()

# --- It has to compile, and it has to reparse. ------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o global_action_emission_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

set(vvp_image ${WORK_DIR}/global_action_emission.vvp)
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${vvp_image} ${OUTPUT_VERILOG} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to compile the regenerated Verilog, exit code ${result}:\n${compile_output}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

execute_process(
    COMMAND ${VVP_EXECUTABLE} ${vvp_image}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE trace
    ERROR_VARIABLE run_errors
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "vvp failed to run, exit code ${result}:\n${run_errors}")
endif()

# --- The behaviour itself, against the hand-computed oracle. ----------------
# s = a and b; t = s; u = s or c; d = (a xor b) delayed 2 ns. The two "_plus1"
# lines are sampled 1 ns after an input change and the "_plus3" lines 3 ns
# after, so d's value differs between them exactly when the delay is honoured.
set(expected_trace
"init             t=0 u=0 d=0
ab_high_plus1    t=1 u=1 d=0
ab_high_plus3    t=1 u=1 d=0
b_low_plus1      t=0 u=0 d=0
b_low_plus3      t=0 u=0 d=1
c_high_plus1     t=0 u=1 d=1
c_high_plus3     t=0 u=1 d=0")

# Drop the "$finish called at ..." line vvp appends, and normalise line endings.
string(REPLACE "\r\n" "\n" trace "${trace}")
string(REGEX REPLACE "\n[^\n]*\\$finish[^\n]*" "" trace "${trace}")
string(STRIP "${trace}" trace)

if(NOT trace STREQUAL expected_trace)
    message(FATAL_ERROR
        "Regenerated Verilog does not reproduce the behaviour of its VHDL source (hif-backend#32).\n"
        "--- expected (hand-computed from the VHDL) ---\n${expected_trace}\n"
        "--- actual ---\n${trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

message(STATUS "global_action_emission test passed.")
