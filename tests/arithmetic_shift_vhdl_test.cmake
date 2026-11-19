# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#94, found in review): hif2vhdl printed the HIF
#           arithmetic shift operators by name, `sra` and `sla`, which numeric_std
#           does not define. Its operators are sll, srl, rol and ror; an
#           arithmetic shift is the function shift_right on a signed value, and
#           shift_left for the left one. vhdl2hif accepts the invalid text, so the
#           values survive a round trip and only the VHDL itself shows it.
#
#           The fixture is the HIF verilog2hif produces for a module shifting a
#           signed value right and left and an unsigned one right (source
#           positions removed).
# -----------------------------------------------------------------------------

foreach(required HIF2VHDL_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${HIF2VHDL_EXECUTABLE} ${FIXTURE} -D ${WORK_DIR}/vhdl_out
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2vhdl failed with exit code ${result}.\n${tool_output}")
endif()

file(GLOB_RECURSE outputs ${WORK_DIR}/vhdl_out/*.vhd)
list(LENGTH outputs output_count)
if(NOT output_count EQUAL 1)
    message(FATAL_ERROR "Expected one VHDL file, found ${output_count}: ${outputs}")
endif()
file(READ ${outputs} vhdl)

if(vhdl MATCHES " sra " OR vhdl MATCHES " sla ")
    message(FATAL_ERROR "An arithmetic shift was printed as an operator numeric_std does not define.\n${vhdl}")
endif()
if(NOT vhdl MATCHES "shift_right\\(s,")
    message(FATAL_ERROR "The arithmetic right shift of the signed port is not shift_right on it.\n${vhdl}")
endif()
if(NOT vhdl MATCHES "shift_left\\(s,")
    message(FATAL_ERROR "The arithmetic left shift of the signed port is not shift_left on it.\n${vhdl}")
endif()

message(STATUS "arithmetic_shift_vhdl test passed.")
