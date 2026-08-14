# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#26): a structure-preserving round trip
#           (`verilog2hif -s`) regenerated the parent's connection nets as
#           `reg` while they were still bound to child instance *output*
#           ports, which Verilog forbids. Icarus rejected the parent with
#           "reg s1; cannot be driven by primitives or continuous assignment".
#
#           The assign-to-always conversion is what promoted them; in the
#           flattened (default) flow that is harmless, because no instance
#           bindings are left.
#
#           Compiling is the check that would have caught the reported bug,
#           but compiling alone would also pass for an output that emits
#           `wire` indiscriminately and so loses the parent's own driven
#           output. The test therefore simulates both the original and the
#           regenerated hierarchy and requires identical traces, the same way
#           sensitivity_equivalence does.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE VVP_EXECUTABLE
                 FIXTURE_PARENT FIXTURE_CHILD TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

# Simulate a parent/child pair together with the shared testbench and return
# its trace. Any non-zero exit is fatal - a design that fails to compile or run
# cannot be compared, and for this regression failing to compile *is* the bug.
function(simulate label parent child out_trace)
    set(vvp_image ${WORK_DIR}/${label}.vvp)
    execute_process(
        COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${vvp_image} ${child} ${parent} ${TESTBENCH}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE compile_output
        ERROR_VARIABLE compile_output
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "iverilog failed to compile ${label}, exit code ${result}:\n${compile_output}")
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

# Step 1: fixtures -> HIF, hierarchy preserved.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -s -o preserved_hierarchy ${FIXTURE_PARENT} ${FIXTURE_CHILD}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif -s) failed with exit code ${result} -- these fixtures are expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/preserved_hierarchy.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# Step 2: HIF -> regenerated Verilog, one file per design unit.
execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_PARENT ${WORK_DIR}/verilog_out/preserved_hierarchy.v)
set(OUTPUT_CHILD ${WORK_DIR}/verilog_out/preserved_hierarchy_child.v)
foreach(expected ${OUTPUT_PARENT} ${OUTPUT_CHILD})
    if(NOT EXISTS ${expected})
        message(FATAL_ERROR "Expected regenerated Verilog not produced: ${expected}")
    endif()
    file(SIZE ${expected} size)
    if(size EQUAL 0)
        message(FATAL_ERROR "Regenerated Verilog is a zero-byte file: ${expected}")
    endif()
endforeach()

file(READ ${OUTPUT_PARENT} parent_content)

# The hierarchy has to still be there - a flattened parent would compile and
# simulate correctly while proving nothing about the `-s` flow.
if(NOT parent_content MATCHES "preserved_hierarchy_child[ \t\n]+u_ha1")
    message(FATAL_ERROR
        "Regenerated parent does not instantiate the child; the hierarchy was not preserved.\nFull content:\n${parent_content}")
endif()

# The reported symptom, checked directly: no net bound to a child output may be
# declared as a variable.
foreach(net s1 c1 c2)
    if(parent_content MATCHES "reg[ \t]+${net}[ \t]*;")
        message(FATAL_ERROR
            "Net '${net}' is bound to a child instance output but declared as reg, which Verilog forbids (hif-backend#26).\nFull content:\n${parent_content}")
    endif()
endforeach()
if(parent_content MATCHES "output[ \t]+reg[ \t]+sum")
    message(FATAL_ERROR
        "Output 'sum' is driven by a child instance output but declared as output reg (hif-backend#26).\nFull content:\n${parent_content}")
endif()

# The converse, so the fix cannot degenerate into emitting nets everywhere:
# cout is driven by the parent's own always block and must stay a variable.
if(NOT parent_content MATCHES "output[ \t]+reg[ \t]+cout")
    message(FATAL_ERROR
        "Output 'cout' is driven by the parent's own logic and must be declared output reg.\nFull content:\n${parent_content}")
endif()

# Step 3: both must compile and simulate identically. Compiling is what the
# reported bug broke; the trace comparison is what keeps the fix honest.
simulate(original ${FIXTURE_PARENT} ${FIXTURE_CHILD} original_trace)
simulate(regenerated ${OUTPUT_PARENT} ${OUTPUT_CHILD} regenerated_trace)

if(NOT original_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated hierarchy is not behaviorally equivalent to its source.\n"
        "--- original trace ---\n${original_trace}\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated parent ---\n${parent_content}")
endif()

# A trace of nothing but x would compare equal to itself and prove nothing.
if(original_trace MATCHES "sum=x" OR original_trace MATCHES "cout=x")
    message(FATAL_ERROR
        "Testbench never resolved the original design's outputs, so the comparison is vacuous:\n${original_trace}")
endif()

message(STATUS "preserved_hierarchy test passed.")
