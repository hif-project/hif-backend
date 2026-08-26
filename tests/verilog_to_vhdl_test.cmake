# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#48): hif2vhdl failed on every verilog2hif
#           output, including the smallest possible one, so the
#           Verilog -> HIF -> VHDL direction did not work at all.
#
#           PreRefine_misc lowers each When into a synthesized when_function_N
#           and inserts it into the nearest enclosing Function or Contents. It
#           walked the whole description, including the standard Verilog
#           library that verilog2hif references in every output. That library
#           declares hif_verilog__system_readmemb with a parameter whose default
#           value is a When, and a When in a SubProgram signature has neither an
#           enclosing Function nor an enclosing Contents, so the pass hit its
#           final else and aborted:
#
#             [HIF2VHDL] [fixMiscIssues] ERROR: Unsupported scope
#               -- in SubProgram: hif_verilog__system_readmemb
#               -- in LibraryDef: hif_verilog_standard
#
#           Exit code 1, no VHDL produced, output directory empty - for any
#           input whatsoever, because the offending When is in the library
#           rather than in the design.
#
#           The fixture also carries a conditional assignment, so that
#           suppressing When lowering outright cannot pass this test: the fix
#           has to skip the standard library *and* still lower user code. That
#           was impossible while hif-backend#54 stood - a Verilog ternary
#           produced an If whose condition was still bit-typed, which VHDL does
#           not allow - and is asserted again now that #54 is fixed.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VHDL_EXECUTABLE VHDL2HIF_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

# --- Setup: Verilog -> HIF ----------------------------------------------

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o verilog_to_vhdl ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to translate cleanly.")
endif()

set(hif_file ${WORK_DIR}/verilog_to_vhdl.hif.xml)
if(NOT EXISTS ${hif_file})
    message(FATAL_ERROR "Expected HIF file not produced: ${hif_file}")
endif()

# The defect is only reachable when the standard library is actually referenced,
# which is what makes this reproducible from any design. If a future frontend
# change stops emitting it, this test would still pass while no longer covering
# anything, so the precondition is asserted rather than assumed.
file(READ ${hif_file} hif_content)
string(FIND "${hif_content}" "hif_verilog_standard" found_at)
if(found_at EQUAL -1)
    message(FATAL_ERROR
        "HIF does not reference hif_verilog_standard, so this test no longer exercises hif-backend#48.")
endif()

# --- The regression: HIF -> VHDL ----------------------------------------

execute_process(
    COMMAND ${HIF2VHDL_EXECUTABLE} ${hif_file} -D ${WORK_DIR}/out
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
    ERROR_VARIABLE stderr_text
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "hif2vhdl failed on verilog2hif output with exit code ${result} (hif-backend#48).\n"
        "Output was:\n${stderr_text}")
endif()

set(generated ${WORK_DIR}/out/src/verilog_to_vhdl.vhd)
if(NOT EXISTS ${generated})
    message(FATAL_ERROR "Expected regenerated VHDL not produced: ${generated}")
endif()
file(SIZE ${generated} size)
if(size EQUAL 0)
    message(FATAL_ERROR "hif2vhdl exited 0 but left a zero-byte file: ${generated}")
endif()

file(READ ${generated} vhdl_content)

# --- The design survived ------------------------------------------------

foreach(expected "ENTITY verilog_to_vhdl" "ARCHITECTURE" "y <= a and b")
    string(FIND "${vhdl_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Regenerated VHDL is missing '${expected}'.\nFull content:\n${vhdl_content}")
    endif()
endforeach()

# User-code When lowering still happens. Without this, a "fix" for hif-backend#48
# that skipped every When rather than only the standard library's would pass
# everything above.
if(NOT vhdl_content MATCHES "when_function")
    message(FATAL_ERROR
        "The conditional assignment was not lowered into a when_function: the hif-backend#48 fix must skip "
        "the standard library without suppressing legitimate When lowering.\nFull content:\n${vhdl_content}")
endif()

# --- The output is real VHDL, not plausible text ------------------------

# No VHDL simulator is assumed here, so the strongest available check is that
# the frontend accepts the result again.
execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o verilog_to_vhdl_reparsed ${generated}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated VHDL failed to reparse (exit code ${result}).\nFull content:\n${vhdl_content}")
endif()

message(STATUS "verilog_to_vhdl test passed.")
