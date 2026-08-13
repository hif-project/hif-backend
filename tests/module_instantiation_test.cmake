# -----------------------------------------------------------------------------
# @brief  : Regression: same root cause as gate_primitives_test.cmake, from a
#           plain (non-primitive) module instantiation instead. Purely
#           combinational submodule instances get flattened by hif-frontend
#           into the same "cone function" mechanism as primitive gates - not
#           a separate Instance/ViewReference code path - so this exercises
#           the identical VerilogPrinter fix from a different Verilog-level
#           construct. See fixtures/module_instantiation.v.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o module_instantiation ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/module_instantiation.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/module_instantiation.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

foreach(expected
    "always @(*)"
    "and_out = a & b"
    "or_out = a | b"
)
    string(FIND "${verilog_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Regenerated Verilog missing expected content: ${expected}\nFull content:\n${verilog_content}")
    endif()
endforeach()

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o module_instantiation_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Regenerated Verilog failed to reparse (exit code ${result}) - this is the actual regression.")
endif()

message(STATUS "module_instantiation test passed.")
