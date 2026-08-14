# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#21): a process's sensitivity list was only
#           partly emitted. HIF keeps level, positive-edge and negative-edge
#           sensitivity in three separate lists; the printer emitted whichever
#           was non-empty first and discarded the rest, and put a single
#           posedge/negedge keyword ahead of a whole comma-separated list
#           instead of attaching one per signal.
#
#           So `always @(posedge clk or negedge rst_n)` came back as
#           `always @(posedge clk)` - an asynchronous reset silently
#           regenerated as a synchronous one - and
#           `always @(posedge clk or posedge rst)` came back as
#           `always @(posedge clk, rst)`, leaving rst sensitive to both edges.
#
#           Like sensitivity_equivalence, this compares simulation traces:
#           the broken output is syntactically perfect and reparses cleanly.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o edge_sensitivity ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/edge_sensitivity.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/edge_sensitivity.v)
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
# Require the asynchronous reset to have actually driven its output.
if(NOT original_trace MATCHES "q_async=0")
    message(FATAL_ERROR
        "Testbench never drove q_async to 0, so the asynchronous-reset comparison is vacuous:
${original_trace}")
endif()

message(STATUS "edge_sensitivity test passed.")
