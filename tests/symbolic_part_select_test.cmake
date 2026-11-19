# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#90): the part-select of a parameter-width
#           vector was dropped.
#
#           `a(WIDTH-2 downto 0)` out of a `WIDTH-1 downto 0` vector is a Cast to
#           a narrower Bitvector, and visitCast could only print the part-select
#           when both widths were numbers. With the width a parameter neither is,
#           so the cast fell through to a bare `a`, one bit wider than asked, and
#           `'0' & a(WIDTH-2 downto 0)` regenerated as `{1'b0, a}`.
#
#           The select is now printed from the bounds, when they are the same
#           symbol and the target's sits lower: `a[width - 2:0]`.
#
#           The fixture is the HIF vhdl2hif produces for a generic-width design
#           (source positions removed). The behavioural half is hif-regression's
#           `parameterized/vhdl_generic_width`.
# -----------------------------------------------------------------------------

foreach(required HIF2VERILOG_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${FIXTURE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result}.\n${tool_output}")
endif()

file(GLOB_RECURSE outputs ${WORK_DIR}/verilog_out/*.v)
list(LENGTH outputs output_count)
if(NOT output_count EQUAL 1)
    message(FATAL_ERROR "Expected one Verilog file, found ${output_count}: ${outputs}")
endif()
file(READ ${outputs} verilog)

if(NOT verilog MATCHES "a\\[width - 2:0\\]")
    message(FATAL_ERROR "The part-select of the parameter-width vector was dropped: expected 'a[width - 2:0]' (hif-backend#90).
")
endif()

message(STATUS "symbolic_part_select test passed.")
