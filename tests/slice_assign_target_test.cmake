# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#103): hif2vhdl aborted on any assignment
#           whose target was a bit-select or a part-select - exit 134 on the
#           Log.cpp assertion "Unexpected target", raised from
#           VHDLPrinter::visitAssign, with the output file already created and
#           left at zero bytes.
#
#           The root cause was in hif-core, not here. visitAssign resolves the
#           target's declaration through hif::getTerminalPrefix and requires
#           the result to be an Identifier. It calls it with the *default*
#           TerminalPrefixOptions, which is value-initialized - deterministic,
#           not undefined behaviour - and hif-core's TerminalPrefixOptions had
#           no default member initializers, so that default was false for every
#           flag despite the header documenting recurseIntoMembers and
#           recurseIntoSlices as true. The call therefore stopped at the Slice
#           or Member instead of recursing to the Identifier underneath it, the
#           dynamic_cast yielded nullptr and the assert fired. hif-core#22 gave
#           the struct the defaults its header documents; this backend needed
#           no change at all, which is why there is no production diff
#           alongside this test.
#
#           Three separate things are checked, because each fails on its own:
#
#             1. hif2vhdl exits 0 - the reported symptom;
#             2. the emitted VHDL is non-empty and every target survives as a
#                slice or bit-select rather than being widened to the whole
#                object - a silent widening would still be valid VHDL;
#             3. vhdl2hif reads the result back.
#
#           (2) carries the weight. An empty file is valid VHDL and reparses
#           cleanly, so exit code and round trip alone would both have scored
#           the broken output as a pass - the same trap recorded in
#           bitselect_target_test.cmake for hif-backend#23.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE VHDL2HIF_EXECUTABLE HIF2VHDL_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o slice_assign_target ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/slice_assign_target.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The targets have to still be a Slice and a Member in the HIF for this to be
# exercising the defect at all. If verilog2hif ever started splitting partial
# assignments itself, hif2vhdl would never see a non-Identifier target and
# every check below would pass vacuously.
file(READ ${HIF_FILE} hif_content)
foreach(target_kind "SLICE" "MEMBER")
    if(NOT hif_content MATCHES "<LEFTHANDSIDE>[ \t\r\n]*<${target_kind}")
        message(FATAL_ERROR
            "verilog2hif produced no ${target_kind} assignment target, so this test is not exercising the "
            "defect it is about (hif-backend#103).")
    endif()
endforeach()

# --- 1. The reported symptom: hif2vhdl aborted here. -------------------------
execute_process(
    COMMAND ${HIF2VHDL_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/vhdl_out
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "hif2vhdl failed with exit code ${result} (expected 0) -- this is the reported symptom of "
        "hif-backend#103, and it is fixed in hif-core rather than here.\nTool output:\n${tool_output}")
endif()

set(OUTPUT_VHDL ${WORK_DIR}/vhdl_out/src/slice_assign_target.vhd)
if(NOT EXISTS ${OUTPUT_VHDL})
    message(FATAL_ERROR "Expected VHDL not produced: ${OUTPUT_VHDL}")
endif()

# --- 2. The actual failure mode: the file exists but is empty. ---------------
file(SIZE ${OUTPUT_VHDL} output_size)
if(output_size EQUAL 0)
    message(FATAL_ERROR
        "Emitted VHDL is zero bytes - hif2vhdl created the file and then aborted "
        "(hif-backend#103).\nTool output:\n${tool_output}")
endif()

file(READ ${OUTPUT_VHDL} vhdl_content)

if(NOT vhdl_content MATCHES "END[ \t]+behav")
    message(FATAL_ERROR "Emitted VHDL has no architecture body.\nFull content:\n${vhdl_content}")
endif()

# Every partial target must survive as a partial target. Widening any of these
# to the whole object would drive bits the design never assigns, and would
# still be valid VHDL that reparses.
foreach(expected "y( 3 downto 0 ) <=" "y( 7 downto 4 ) <=" "z( 0 ) <=" "z( 7 ) <=")
    string(FIND "${vhdl_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Emitted VHDL is missing the assignment target '${expected}'.\nFull content:\n${vhdl_content}")
    endif()
endforeach()

# --- 3. And the result must be readable back. --------------------------------
execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o slice_assign_target_reparsed ${OUTPUT_VHDL}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE tool_output
    ERROR_VARIABLE tool_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "The emitted VHDL failed to reparse (exit code ${result}).\n${tool_output}\nFull content:\n${vhdl_content}")
endif()

message(STATUS "slice_assign_target test passed.")
