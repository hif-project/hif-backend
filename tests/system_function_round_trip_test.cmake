# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#19): Verilog system functions did not
#           survive a round trip.
#
#           verilog2hif renames "$clog2" to "_system_clog2" and
#           standardization prefixes it to "hif_verilog__system_clog2". The
#           printer emitted that internal name verbatim, which is not a
#           callable function in Verilog: no simulator accepts it, and the
#           frontend has no declaration to bind it to, so reparsing the
#           regenerated design asserted.
#
#           hif2verilog exits 0 either way, so the checks that matter are
#           that the '$' spelling is back and that the output is valid input
#           to verilog2hif again.
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
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o system_function_round_trip ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/system_function_round_trip.hif.xml)
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

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/system_function_round_trip.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# The internal name must not reach the output at all.
if(verilog_content MATCHES "_system_clog2")
    message(FATAL_ERROR
        "The internal name of $clog2 was emitted instead of '$clog2' (hif-backend#19).\nFull content:\n${verilog_content}")
endif()

# Both uses in the fixture - the port width and the body expression - must be
# back to the '$' spelling.
if(NOT verilog_content MATCHES "\\[\\$clog2\\(DEPTH\\) - 1:0\\]")
    message(FATAL_ERROR
        "$clog2 is not emitted in the port widths (hif-backend#19).\nFull content:\n${verilog_content}")
endif()
if(NOT verilog_content MATCHES "seed \\* \\$clog2\\(DEPTH\\)")
    message(FATAL_ERROR
        "$clog2 is not emitted in the body expression (hif-backend#19).\nFull content:\n${verilog_content}")
endif()

# The property the fix is really about: the regenerated design is consumable
# again. This is the step that asserted before the fix.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o system_function_round_trip_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}) - this is the actual regression.\nFull content:\n${verilog_content}")
endif()

message(STATUS "system_function_round_trip test passed.")
