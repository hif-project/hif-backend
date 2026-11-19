# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#87): an unpacked array was printed as a
#           vector, with its element width lost.
#
#           `reg [7:0] mem [0:3]` is an array of four 8-bit words. getBitwidth
#           took the array's span as the vector width, so it came out as
#           `reg [0:3] mem = 08'bxxxxxxxx18'bxxxxxxxx...` - four bits, and an
#           initializer made of the words' default values run together, which is
#           not Verilog. The element width now goes before the name, the array
#           range after it, and no initializer is printed: Verilog-2001 has none
#           for an unpacked array.
#
#           A process that reads the array was also given `@(mem, raddr)`, which
#           Verilog rejects for a memory, so it is printed as `@(*)`.
#
#           The fixture is the HIF verilog2hif produces for a four-word register
#           file (source positions removed). hif-regression's
#           `sequential/reg_array_sync_write` is the structural half.
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

if(NOT verilog MATCHES "reg \\[7:0\\] mem \\[0:3\\];")
    message(FATAL_ERROR "Expected the array as 'reg [7:0] mem [0:3];' (hif-backend#87).
")
endif()

if(verilog MATCHES "always @\\( mem")
    message(FATAL_ERROR "A memory is in an event list, which Verilog rejects (hif-backend#87).
")
endif()

message(STATUS "unpacked_array test passed.")
