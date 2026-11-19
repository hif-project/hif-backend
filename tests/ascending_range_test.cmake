# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#91): an ascending range was treated as
#           descending.
#
#           `std_logic_vector(0 to 3)` has element 0 on the left. Printed as
#           `[3:0]` it puts element 0 on the right, so `a(0)` and `a(3)` read
#           each other's bit. And a literal's size was computed as left - right
#           + 1, which is negative for an ascending range: `-2'b1100`.
#
#           Two things are checked: the port keeps its direction, `[0:3]`, and
#           the literal is `4'b1100` with a positive size.
#
#           The fixture is the HIF vhdl2hif produces for a small design with one
#           ascending port and one ascending constant (source positions
#           removed). The behavioural half is hif-regression's
#           `combinational/vhdl_ascending_range` and `vhdl_ascending_literal`.
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

if(NOT verilog MATCHES "input wire \\[0:3\\] a")
    message(FATAL_ERROR "The ascending port lost its direction: expected 'input wire [0:3] a' (hif-backend#91).\n${verilog}")
endif()

if(verilog MATCHES "-[0-9]+'b")
    message(FATAL_ERROR "A literal was printed with a negative size (hif-backend#91).\n${verilog}")
endif()
if(NOT verilog MATCHES "4'b1100")
    message(FATAL_ERROR "Expected the constant as 4'b1100 (hif-backend#91).\n${verilog}")
endif()

message(STATUS "ascending_range test passed.")
