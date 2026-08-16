# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#51): a VHDL clocked process regenerated as
#           a call to a function that does not exist.
#
#           VHDL has no edge-sensitive sensitivity list. A clocked process is
#           sensitive to the clock's level and tests the edge in its body, with
#           `clk'event and clk = '1'` or with rising_edge/falling_edge. Verilog
#           has the opposite arrangement: the edge belongs in the sensitivity
#           list and there is no way to spell the test where it stands.
#
#           hif2verilog printed the test verbatim, so a flip-flop came back as
#
#               always @( clk, rst )
#                   ...
#                   else if ( hif_vhdl_event() && clk === 1 )
#
#           at exit code 0. `hif_vhdl_event` is declared nowhere, and the call
#           carries no argument, so not even a reader could tell which signal's
#           edge was meant. With the rising_edge spelling the condition came
#           out empty instead (hif-backend#50), so between the two, no VHDL
#           sequential design survived the trip to Verilog.
#
#           The fix rebuilds the sensitivity list from hif-core's process
#           analysis. This test therefore has to prove *behaviour*, not shape:
#           the output has to be a flip-flop, not merely compile. A rebuild
#           that produced `always @(clk, rst)` - level sensitive - would still
#           compile, still reparse, and still look plausible.
#
#           So the design is simulated and its trace compared against an oracle
#           computed by hand from the VHDL. Two samples carry the argument:
#
#             t20  d has moved while the clock is idle. Every output must still
#                  hold its old value. A level-sensitive rebuild follows d here.
#             t36  rst is asserted while the clock is idle. q_async must fall at
#                  once and q_sync must not - the difference between an
#                  asynchronous and a synchronous reset, and what a rebuild that
#                  promoted the synchronous reset into the sensitivity list
#                  would get wrong.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE VVP_EXECUTABLE FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

# --- Setup: VHDL -> HIF -------------------------------------------------

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o clocked_process ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(hif_file ${WORK_DIR}/clocked_process.hif.xml)
if(NOT EXISTS ${hif_file})
    message(FATAL_ERROR "Expected HIF file not produced: ${hif_file}")
endif()

# --- HIF -> Verilog -----------------------------------------------------

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${hif_file} -D ${WORK_DIR}/out
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
    ERROR_VARIABLE stderr_text
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "hif2verilog failed with exit code ${result} (hif-backend#51).\nOutput was:\n${stderr_text}")
endif()

set(generated ${WORK_DIR}/out/clocked_process.v)
if(NOT EXISTS ${generated})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${generated}")
endif()

file(READ ${generated} verilog_content)

# --- No internal name survived into the output --------------------------

# The reported symptom, checked directly. Any hif_vhdl_* name in Verilog is a
# HIF-internal identifier that escaped, whether or not it happens to compile.
string(REGEX MATCHALL "hif_vhdl_[A-Za-z0-9_]*" leaked_names "${verilog_content}")
if(leaked_names)
    list(REMOVE_DUPLICATES leaked_names)
    string(REPLACE ";" ", " leaked_list "${leaked_names}")
    message(FATAL_ERROR
        "Regenerated Verilog still names HIF-internal function(s): ${leaked_list} (hif-backend#51).\n"
        "Full content:\n${verilog_content}")
endif()

# --- The edges reached the sensitivity list -----------------------------

foreach(expected "posedge clk" "negedge clk" "posedge rst")
    string(FIND "${verilog_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Regenerated Verilog has no '${expected}' in any sensitivity list.\nFull content:\n${verilog_content}")
    endif()
endforeach()

# --- Behaviour ----------------------------------------------------------

set(vvp_image ${WORK_DIR}/clocked_process.vvp)
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${vvp_image} ${generated} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog did not compile (exit ${result}):\n${compile_output}\n"
        "Full content:\n${verilog_content}")
endif()

execute_process(
    COMMAND ${VVP_EXECUTABLE} ${vvp_image}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE trace
    ERROR_VARIABLE trace_err
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Simulation failed (exit ${result}):\n${trace}${trace_err}")
endif()

# The oracle, computed by hand from clocked_process.vhd. Stated as whole lines
# so that a value changing at the wrong *time* fails just as loudly as a wrong
# value: it is the timing that distinguishes a flip-flop from a latch.
#
# The first three samples deliberately omit q_fall. Verilog counts a transition
# out of x as an edge and VHDL's falling_edge() does not, so before q_fall has
# seen a real 1 -> 0 edge the two languages legitimately disagree there -
# asserting the Verilog value would be stating a Verilog result as if it had
# been derived from the VHDL. From t31 on they agree and it is checked.
set(expected_trace
    "t05_reset_no_edge       q_async=0 q_sync=x"
    "t20_d_high_no_edge      q_async=0 q_sync=x"
    "t26_after_posedge       q_async=1 q_sync=1"
    "t31_after_negedge       q_async=1 q_sync=1 q_fall=1"
    "t36_async_rst_no_edge   q_async=0 q_sync=1 q_fall=1"
    "t41_after_posedge_rst   q_async=0 q_sync=0 q_fall=1"
    "t51_after_negedge_d0    q_async=0 q_sync=0 q_fall=0"
    "t56_after_posedge_d0    q_async=0 q_sync=0 q_fall=0"
)

foreach(line ${expected_trace})
    string(FIND "${trace}" "${line}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Simulation trace does not contain the expected line:\n  ${line}\n"
            "Full trace:\n${trace}\n"
            "Regenerated Verilog:\n${verilog_content}")
    endif()
endforeach()

message(STATUS "clocked_process test passed.")
