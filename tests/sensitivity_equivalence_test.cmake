# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#16): regenerated Verilog was not
#           behaviorally equivalent to its source. hif-frontend factors shared
#           combinational sub-expressions into "cone" Procedures that the
#           calling process invokes inline (a ProcedureCall in its body), and
#           gives the process a sensitivity list over the cone's transitive
#           primary inputs - which is correct, because in HIF the cone is
#           re-evaluated inside the caller.
#
#           hif2verilog instead hoisted each cone into its own
#           `always @(*)` block and dropped the call, turning an
#           intra-process dependency into an inter-process one that nothing
#           orders. The reader was left sensitive to a, b but reading t.
#
#           Unlike the other tests here, this one is not a text check: the
#           broken output parses and reparses cleanly. It compares the actual
#           simulation trace of the fixture against the trace of the
#           regenerated Verilog, using one testbench compiled against each.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE VVP_EXECUTABLE FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

# Simulate a given Verilog source together with the shared testbench, and
# return its trace. Any non-zero exit is fatal - a design that fails to
# compile or run cannot be compared.
function(simulate label source out_trace)
    set(vvp_image ${WORK_DIR}/${label}.vvp)
    execute_process(
        COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${vvp_image} ${source} ${TESTBENCH}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE compile_output
        ERROR_VARIABLE compile_output
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "iverilog failed to compile ${label} (${source}), exit code ${result}:\n${compile_output}")
    endif()

    execute_process(
        COMMAND ${VVP_EXECUTABLE} ${vvp_image}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE trace
        ERROR_VARIABLE run_errors
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "vvp failed to run ${label}, exit code ${result}:\n${run_errors}")
    endif()

    string(STRIP "${trace}" trace)
    set(${out_trace} "${trace}" PARENT_SCOPE)
endfunction()

# Step 1: fixture -> HIF.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o sensitivity_equivalence ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/sensitivity_equivalence.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# Step 2: HIF -> regenerated Verilog.
execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/sensitivity_equivalence.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

# Step 3: simulate both and require identical traces.
simulate(original ${FIXTURE} original_trace)
simulate(regenerated ${OUTPUT_VERILOG} regenerated_trace)

if(NOT original_trace STREQUAL regenerated_trace)
    file(READ ${OUTPUT_VERILOG} verilog_content)
    message(FATAL_ERROR
        "Regenerated Verilog is not behaviorally equivalent to its source.\n"
        "--- original trace ---\n${original_trace}\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

# A trace of nothing but x would compare equal to itself and prove nothing.
# Require the original to have actually resolved.
if(original_trace MATCHES "sum=x" OR original_trace MATCHES "cout=x")
    message(FATAL_ERROR
        "Testbench never resolved the original design's outputs, so the comparison is vacuous:\n${original_trace}")
endif()

message(STATUS "sensitivity_equivalence test passed.")
