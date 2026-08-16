# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#46): a VHDL process ending in `wait;` -
#           suspend permanently - regenerated as a bare `always begin ... end`,
#           a zero-delay infinite loop that Icarus rejects at elaboration, with
#           the `wait;` itself dropped. Exit code 0.
#
#           This is the failure class hif-backend#40 fixed for a process with
#           no wait at all. It survived that fix because isRetriggerable
#           counted *any* Wait as a way to be woken up, including an empty one,
#           which by definition is not.
#
#           Underneath was a representation problem rather than a printing one.
#           An empty Wait had two producers meaning opposite things: vhdl2hif's
#           `wait;`, suspend forever, and the marker verilog2hif appends to
#           every suspending process to record that it loops back round, which
#           the SystemC lowering emits at the tail of its `while (true)`.
#           Nothing on the node told them apart, so hif2verilog could not choose
#           and printed nothing - right for the marker, wrong for `wait;`.
#
#           verilog2hif now tags the marker PROPERTY_PROCESS_LOOP_TAIL, so this
#           side can tell them apart. That is why .ci/pinned-refs.env moves
#           HIF_FRONTEND_REF: against a frontend that does not tag, the marker
#           is indistinguishable from `wait;` again and every Verilog process
#           containing a wait would be demoted to `initial` - silently stopping
#           the design after its first pass, which is worse than the defect
#           being fixed. The existing wait_emission and initial_process tests
#           cover that direction.
#
#           Compiling is not the property under test: a process emitted as
#           `always` fails to elaborate, which would be caught anyway. The
#           stimulus proves the processes actually *stopped* - `a` changes
#           after they have run, and a process that looped would pick it up.
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
    COMMAND ${VHDL2HIF_EXECUTABLE} -o permanent_suspension ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(hif_file ${WORK_DIR}/permanent_suspension.hif.xml)
if(NOT EXISTS ${hif_file})
    message(FATAL_ERROR "Expected HIF file not produced: ${hif_file}")
endif()

# The VHDL `wait;` must reach the backend as an *untagged* empty Wait, which is
# the whole basis of the distinction. If vhdl2hif ever started tagging it, or
# stopped emitting it, this test would keep passing while covering nothing.
file(READ ${hif_file} hif_content)
string(FIND "${hif_content}" "PROPERTY_PROCESS_LOOP_TAIL" found_at)
if(NOT found_at EQUAL -1)
    message(FATAL_ERROR
        "vhdl2hif tagged a Wait with PROPERTY_PROCESS_LOOP_TAIL. That property marks the loop-tail "
        "marker verilog2hif synthesizes, and a VHDL `wait;` is not one, so this test no longer "
        "exercises hif-backend#46.")
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
        "hif2verilog failed with exit code ${result} (hif-backend#46).\nOutput was:\n${stderr_text}")
endif()

set(generated ${WORK_DIR}/out/permanent_suspension.v)
if(NOT EXISTS ${generated})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${generated}")
endif()

file(READ ${generated} verilog_content)

# --- Both processes run once ---------------------------------------------

# Structural, and the only cover p_once gets: its output is assigned in the
# same time step the testbench drives its input, so its *value* is a race and
# asserting one would be asserting the race rather than the translation.
string(REGEX MATCHALL "always" always_blocks "${verilog_content}")
list(LENGTH always_blocks always_count)
if(always_count GREATER 0)
    message(FATAL_ERROR
        "Regenerated Verilog contains ${always_count} `always` block(s); a process that suspends "
        "permanently runs once and must be `initial` (hif-backend#46).\n"
        "Full content:\n${verilog_content}")
endif()

string(REGEX MATCHALL "initial" initial_blocks "${verilog_content}")
list(LENGTH initial_blocks initial_count)
if(initial_count LESS 2)
    message(FATAL_ERROR
        "Regenerated Verilog has ${initial_count} `initial` block(s); expected 2, one per process.\n"
        "Full content:\n${verilog_content}")
endif()

# --- Behaviour: they really stopped --------------------------------------

set(vvp_image ${WORK_DIR}/permanent_suspension.vvp)
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${vvp_image} ${generated} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog did not compile or elaborate (exit ${result}):\n${compile_output}\n"
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

# The oracle, computed by hand from permanent_suspension.vhd.
#
# t15 vs t05 is the 10 ns delay being honoured. t35 and t55 are the point of
# the test: `a` went to 0 at t20, and `late` must not follow, because p_delay
# suspended for good rather than going back round.
set(expected_trace
    "t05_before_delay    late=0"
    "t15_after_delay     late=1"
    "t35_a_changed       late=1"
    "t55_still_suspended late=1"
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

message(STATUS "permanent_suspension test passed.")
