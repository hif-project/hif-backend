# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#36): a VHDL output port declared with an
#           explicit initial value lost it when a process drove the port.
#           hif2verilog emitted `output reg q` with no initialization, so the
#           regenerated design read x for the whole interval before the driving
#           process first wrote the port. Exit 0, and the output compiled and
#           reparsed cleanly.
#
#           This is the other half of the field hif-backend#30 covers. There
#           the port has no other driver and the value stands in for a folded
#           constant continuous assignment, so it is re-emitted as `assign`.
#           Here the port IS driven, procedurally, so it is a reg: a continuous
#           assign would be a second driver on it, and the value is what the
#           port holds until that process first writes it - an `initial`
#           assignment. The two cases are complementary and the gate that
#           separates them is load-bearing, because vhdl2hif gives every out
#           port a VALUE (the 'U' default) whether the source wrote one or not.
#
#           There is no VHDL simulator here (no ghdl/nvc, locally or in CI), so
#           the source cannot be simulated for comparison. The oracle is
#           computed by hand from the VHDL and stated literally below.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE VERILOG2HIF_EXECUTABLE
                 IVERILOG_EXECUTABLE VVP_EXECUTABLE FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o port_initial_process ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/port_initial_process.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The premise: the frontend records the stated value on the port. If it did
# not, this would be a vhdl2hif issue and not an emission one.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "BITVAL value=\"1\"")
    message(FATAL_ERROR
        "vhdl2hif did not record the ports' stated initial values, so this test is not exercising the emission "
        "gap it is about. The frontend, not hif2verilog, would be at fault.\n")
endif()
# And it gives the port that states nothing the 'U' default, which is exactly
# what makes "emit every out port's value" the wrong rule.
if(NOT hif_content MATCHES "BITVAL value=\"U\"")
    message(FATAL_ERROR
        "The fixture produced no 'U'-valued port, so the control that separates a stated initial value from the "
        "default is not present and the gate below is untested.\n")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/port_initial_process.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- The value is emitted, procedurally. -------------------------------------
if(NOT verilog_content MATCHES "initial begin[ \t\r\n]+q = ")
    message(FATAL_ERROR
        "The process-driven port's stated initial value was not emitted as an `initial` assignment "
        "(hif-backend#36).\nFull content:\n${verilog_content}")
endif()

# --- And not as a second driver on the port the concurrent assign owns. ------
# t is driven continuously, so it is a wire. A procedural assignment to it does
# not compile, and a second continuous assign would resolve to x.
if(verilog_content MATCHES "initial begin[^e]*t = ")
    message(FATAL_ERROR
        "A continuously driven port's initial value was restated procedurally. That port is a net "
        "(hif-backend#30/#32).\nFull content:\n${verilog_content}")
endif()
if(NOT verilog_content MATCHES "assign t = ")
    message(FATAL_ERROR
        "The concurrently driven port lost its continuous assignment (hif-backend#32).\n"
        "Full content:\n${verilog_content}")
endif()

# --- The port that stated no value gets nothing. -----------------------------
# vhdl2hif gave s a VALUE too - 'U'. Emitting it would either produce
# "s = ;" or drive an x onto a port the source left alone.
if(verilog_content MATCHES "[ \t]s = ")
    message(FATAL_ERROR
        "A port whose only value is the 'U' default was initialized. That default is not something the source "
        "wrote (hif-backend#36).\nFull content:\n${verilog_content}")
endif()

# Nor may any all-unknown value be emitted. vhdl2hif's 'U' has no Verilog
# literal and renders empty, but verilog2hif supplies an all-x value for a reg
# port instead, which does render - so an emptiness check alone lets "q =
# 4'bxxxx;" through. It says nothing an uninitialized reg does not already say.
if(verilog_content MATCHES "= [0-9]*'[bhod][xXzZ_]+;")
    message(FATAL_ERROR
        "An all-unknown initial value was emitted. Both frontends supply one for a port that stated none, so it "
        "is not something the source wrote (hif-backend#36).\nFull content:\n${verilog_content}")
endif()

# --- Reparse. ----------------------------------------------------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o port_initial_process_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

# --- Simulate against the hand-computed oracle. ------------------------------
# Compilation is itself part of the check: had the fix initialized t, which the
# concurrent assignment drives, iverilog would reject the procedural assignment
# to a net here.
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/regenerated.vvp ${OUTPUT_VERILOG} ${TESTBENCH}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog does not compile, exit code ${result}:\n${compile_output}\n"
        "Full content:\n${verilog_content}")
endif()

execute_process(
    COMMAND ${VVP_EXECUTABLE} ${WORK_DIR}/regenerated.vvp
    RESULT_VARIABLE result
    OUTPUT_VARIABLE trace
    ERROR_VARIABLE run_errors
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "vvp failed to run the regenerated design, exit code ${result}:\n${run_errors}")
endif()

string(REPLACE "\r\n" "\n" trace "${trace}")

# Oracle, read off the VHDL by hand:
#
#   t=10  a='0'. Signals are initialized (q='1', s='U'), then every process
#         runs once. The process's condition is false, so it does not write q,
#         and q is still the '1' the entity declared. The concurrent assignment
#         drives t from a, so t='0'. s is written by nothing and stays unknown.
#           -> q=1 s=x t=0
#   t=20  a='1'. The process runs and writes q<='0'; t follows a to '1'.
#           -> q=0 s=x t=1
#   t=30  a='0'. The process runs, the condition is false again, so q keeps the
#         '0' it was last given - it does not revert to the initial value. t
#         follows a back to '0'.
#           -> q=0 s=x t=0
#
# The first line is the one that fails before the fix, with q=x.
if(NOT trace MATCHES "t=10 a=0 q=1 s=x t=0")
    message(FATAL_ERROR
        "The regenerated design does not hold the port's stated initial value before the driving process first "
        "writes it (hif-backend#36).\nExpected 't=10 a=0 q=1 s=x t=0'.\n--- trace ---\n${trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()
if(NOT trace MATCHES "t=20 a=1 q=0 s=x t=1")
    message(FATAL_ERROR
        "The driving process no longer overwrites the initial value. Emitting the value must not stop the "
        "process writing the port (hif-backend#36).\nExpected 't=20 a=1 q=0 s=x t=1'.\n--- trace ---\n${trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()
if(NOT trace MATCHES "t=30 a=0 q=0 s=x t=0")
    message(FATAL_ERROR
        "The port reverted to its initial value after the process had written it. The value initializes the "
        "port once, it does not hold it (hif-backend#36).\nExpected 't=30 a=0 q=0 s=x t=0'.\n"
        "--- trace ---\n${trace}\n--- regenerated source ---\n${verilog_content}")
endif()

message(STATUS "port_initial_process test passed.")
