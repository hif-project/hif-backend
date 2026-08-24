# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#71): a VHDL `inout` port that a process
#           drives was emitted as a bare Verilog `inout` - a net - while the
#           process assigned to it procedurally:
#
#               module inout_port(input wire a, inout b);
#                   always @( a ) begin
#                       b <= a;       <-- b is a net, not a valid l-value
#                   end
#
#           `dir_out` already consulted isContinuouslyDriven to choose between
#           `output wire` and `output reg` (hif-backend#26, #32); `dir_inout`
#           consulted nothing. Verilog-2001 has no `inout reg`, so unlike the
#           `dir_out` case it could not be settled by choosing a keyword - it
#           needs a lowering, which lowerProcedurallyDrivenInoutPorts now does.
#
#           Needs iverilog, and specifically its *elaboration*: the defect is
#           not a parse error. hif2verilog exited 0, `verilog2hif` reparsed the
#           output cleanly, and only elaboration ever said anything - which is
#           why the round-trip gate this project uses did not catch it.
#
#           Needs a simulator as well, for the three properties a lowering can
#           get wrong while still elaborating:
#
#             - releasing the net. VHDL says high impedance with a *value*,
#               'Z', so the continuous driver can be unconditional; a lowering
#               that drove unconditionally *and* dropped 'Z' would still
#               elaborate.
#             - reading the net. A process's read of the port must not follow
#               its write onto the driver reg, or the module reads back what it
#               last drove rather than what is on the wire. Silent: it compiles
#               either way and only differs when someone else drives.
#             - leaving a concurrently driven inout alone. A net is already
#               right there, and a second driver would be a real defect.
#
#           Observed against the pre-fix binary, which is what says this test
#           discriminates. The shape check below fires first, since it is the
#           cheaper diagnostic:
#
#               The regenerated Verilog declares no driver reg for the
#               procedurally driven inout 'b', so it is still assigning to a net
#
#           and running iverilog on that same pre-fix output directly gives the
#           symptom as reported:
#
#               inout_port_driver.v:18: error: b is not a valid l-value in
#                 inout_port_driver_tb.dut.
#               inout_port_driver.v:9:      : b is declared here as wire.
#               2 error(s) during elaboration.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE VERILOG2HIF_EXECUTABLE IVERILOG_EXECUTABLE
                 VVP_EXECUTABLE FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o inout_port_driver ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/inout_port_driver.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# Both inout ports have to survive as dir_inout, or the test stops covering
# either the lowered case or its control.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "<PORT[^>]*direction=\"INOUT\"[^>]*name=\"b\"")
    message(FATAL_ERROR
        "vhdl2hif produced no dir_inout Port named 'b', so this test is not exercising the case it "
        "is about (hif-backend#71).")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/inout_port_driver.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- Shape: the lowering happened, and only where it was needed. -------------
if(NOT verilog_content MATCHES "reg[ \t]+b_drv")
    message(FATAL_ERROR
        "The regenerated Verilog declares no driver reg for the procedurally driven inout 'b', so it "
        "is still assigning to a net (hif-backend#71).\nFull content:\n${verilog_content}")
endif()

if(NOT verilog_content MATCHES "assign[ \t]+b[ \t]*=[ \t]*b_drv")
    message(FATAL_ERROR
        "The driver reg is declared but never connected to the inout port, so the port is undriven "
        "(hif-backend#71).\nFull content:\n${verilog_content}")
endif()

# 'c' is driven by a concurrent assignment, so a net is already right and it
# must not have acquired a driver reg of its own.
if(verilog_content MATCHES "reg[ \t]+c_drv")
    message(FATAL_ERROR
        "A concurrently driven inout was given a driver reg it does not need, which puts a second "
        "driver on the net (hif-backend#71).\nFull content:\n${verilog_content}")
endif()

# --- Reparse: the frontend has to accept what the backend wrote. -------------
# Passed before the fix too - the defect was never a parse error - so this is
# here to keep the lowering inside what the frontend reads, not to show #71.
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o inout_port_driver_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- Elaboration: the reported symptom. --------------------------------------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/inout_port_driver.vvp
            ${OUTPUT_VERILOG} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "iverilog failed to elaborate the regenerated design, exit code ${result} -- which is the "
        "reported symptom of hif-backend#71:\n${compile_output}\nFull content:\n${verilog_content}")
endif()

# --- The behaviour elaboration alone does not pin. ---------------------------
execute_process(
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/inout_port_driver.vvp
    RESULT_VARIABLE result
    OUTPUT_VARIABLE trace
    ERROR_VARIABLE run_errors
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "vvp failed to run, exit code ${result}:\n${run_errors}")
endif()

string(REPLACE "\r\n" "\n" trace "${trace}")

if(trace MATCHES "FAIL")
    message(FATAL_ERROR
        "The regenerated design elaborates but does not behave as a bidirectional port: it either "
        "fails to release the net, or reads back its own driver instead of the net "
        "(hif-backend#71).\n--- trace ---\n${trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

if(NOT trace MATCHES "ALL CHECKS PASSED")
    message(FATAL_ERROR
        "The testbench did not report completion, so the comparison proves nothing.\n"
        "--- trace ---\n${trace}")
endif()

message(STATUS "inout_port_driver test passed.")
