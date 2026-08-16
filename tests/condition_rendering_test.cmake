# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#50): VerilogPrinter::getValue rendered a
#           value whose kind was absent from its dynamic_cast chain as the
#           empty string, and said nothing about it.
#
#           getValue renders every if/else-if condition, every case selector
#           and label, and the for condition - positions where "nothing" is not
#           a legal spelling. A FunctionCall was one of the kinds missing from
#           the chain, so `if rising_edge(clk)` regenerated as:
#
#               if (  ) begin
#
#           at exit code 0. Not mis-rendered: absent. iverilog reports a plain
#           syntax error and verilog2hif aborts on the way back in, so the
#           output was unusable while the tool reported success.
#
#           What makes this worth a test beyond the one symptom is the
#           asymmetry it came from. An Expression *was* handled, by delegating
#           to renderToString, so `clk'event and clk = '1'` printed its call
#           while a bare `rising_edge(clk)` printed nothing - the same
#           construct behaving differently depending on whether something else
#           wrapped it. The fix gives the chain a terminal case that delegates
#           the same way, so the default for a new value kind is "render it"
#           rather than "drop it".
#
#           Scope: this asserts the condition survives, NOT that the result
#           compiles. It does not - `hif_vhdl_rising_edge` is not a Verilog
#           function, which is hif-backend#51, a separate defect on the same
#           reproducer. Before this fix the condition was gone entirely; after
#           it, the condition is present and names what it calls. Asserting
#           compilation here would be asserting #51, and this test would then
#           fail for a reason it is not about.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

# --- Setup: VHDL -> HIF -------------------------------------------------

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o condition_rendering ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(hif_file ${WORK_DIR}/condition_rendering.hif.xml)
if(NOT EXISTS ${hif_file})
    message(FATAL_ERROR "Expected HIF file not produced: ${hif_file}")
endif()

# The defect was in rendering, not in the frontend: the HIF carried the calls
# all along. Asserted so that a future frontend change that stopped emitting
# them would be reported as itself, rather than silently turning this into a
# test that proves nothing.
file(READ ${hif_file} hif_content)
string(REGEX MATCHALL "<FCALL[^>]*name=\"hif_vhdl_(rising|falling)_edge\"" hif_calls "${hif_content}")
list(LENGTH hif_calls hif_call_count)
if(hif_call_count LESS 2)
    message(FATAL_ERROR
        "HIF carries ${hif_call_count} edge-function call(s); expected 2. "
        "This test no longer exercises hif-backend#50.")
endif()

# --- The regression: HIF -> Verilog -------------------------------------

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${hif_file} -D ${WORK_DIR}/out
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(generated ${WORK_DIR}/out/condition_rendering.v)
if(NOT EXISTS ${generated})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${generated}")
endif()

file(READ ${generated} verilog_content)

# --- No empty condition anywhere ----------------------------------------

# The exact defect, matched as structure rather than as a substring: an `if`
# or `else if` whose parentheses contain nothing but whitespace. Anchored this
# way so it cannot pass by coincidence on some unrelated formatting change.
string(REGEX MATCHALL "if[ \t]*\\([ \t]*\\)" empty_conditions "${verilog_content}")
list(LENGTH empty_conditions empty_count)
if(empty_count GREATER 0)
    message(FATAL_ERROR
        "Regenerated Verilog contains ${empty_count} empty branch condition(s) -- the condition was "
        "dropped rather than rendered (hif-backend#50).\nFull content:\n${verilog_content}")
endif()

# --- Both conditions are present, and are the right ones ----------------

# Counting matters: visitIf renders the first alternative and the later ones
# through separate getValue calls, so a fix reaching only the first would leave
# the elsif empty and still satisfy a check that just looked for "clk".
foreach(expected "rising_edge" "falling_edge")
    string(FIND "${verilog_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Regenerated Verilog does not mention '${expected}', so that branch's condition was not "
            "rendered.\nFull content:\n${verilog_content}")
    endif()
endforeach()

# And that each condition carries its own operand, so that rendering the call
# but losing its argument does not pass.
foreach(expected_pair "rising_edge(clk)" "falling_edge(rst)")
    string(FIND "${verilog_content}" "${expected_pair}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Regenerated Verilog does not contain '${expected_pair}', so the call was rendered "
            "without its operand.\nFull content:\n${verilog_content}")
    endif()
endforeach()

message(STATUS "condition_rendering test passed.")
