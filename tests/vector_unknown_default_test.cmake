# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#55): a VHDL vector port with no stated
#           initial value regenerated as `initial vec = 4'buuuu;`. `u` is not a
#           Verilog binary digit, so the output did not parse - iverilog reports
#           a syntax error and verilog2hif aborts on the way back in.
#
#           Two things were wrong and both had to be fixed together. The 'U' is
#           vhdl2hif's default for "the source said nothing", so hif-backend#36
#           requires it not to be emitted at all; and hif2verilog's four-state
#           literal branch assumed the IEEE-1164 alphabet was spellable in
#           Verilog, which is true for X and Z but not for U/W/L/H/-. Spelling
#           it legally would still have emitted a value the source never wrote.
#
#           The scalar path was already correct, by structure rather than by
#           decision: getValue renders only is01() bit values and the empty
#           result makes the port be skipped. The fix gives the vector path the
#           same property, so this test holds the two to the same rule.
#
#           Exit code was 0 throughout, so the check is on the emitted text and
#           on whether the result actually builds.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE VERILOG2HIF_EXECUTABLE IVERILOG_EXECUTABLE
                 FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o vector_unknown_default ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (vhdl2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/vector_unknown_default.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

# The frontend has to have supplied the implicit 'U' default for this test to be
# about emission at all. If vhdl2hif ever stops giving an unstated port that
# default, the defect becomes unreachable and every check below would pass
# vacuously.
file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "BITVECTORVAL VALUE=\"UUUU\"")
    message(FATAL_ERROR
        "vhdl2hif did not give the unstated vector port its implicit 'U' default, so this test is not "
        "exercising the emission gap it is about (hif-backend#55).")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/vector_unknown_default.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

# --- No IEEE-1164 metavalue may appear inside a Verilog literal. -------------
# Anchored on the literal's own syntax rather than on the string "4'buuuu", so
# that any width, and W/L/H as well as U, are caught.
if(verilog_content MATCHES "'[bB][01xXzZ_]*[uUwWlLhH]")
    message(FATAL_ERROR
        "Regenerated Verilog contains a literal spelled with an IEEE-1164 metavalue Verilog does not "
        "define; the file will not parse (hif-backend#55).\nFull content:\n${verilog_content}")
endif()

# --- The implicit default must not be emitted, at either width. --------------
# The scalar is the control: it was already correct, and holding both to the
# same assertion is the point of the fixture.
foreach(port sc vec)
    if(verilog_content MATCHES "[ \t]${port} = ")
        message(FATAL_ERROR
            "Output port '${port}' states no initial value in the VHDL, but the regenerated Verilog "
            "initializes it: the frontend's implicit 'U' default was emitted as though the source had "
            "asked for it (hif-backend#55/#36).\nFull content:\n${verilog_content}")
    endif()
endforeach()

# --- A stated initial value must still survive. ------------------------------
# Without this the defect could be "fixed" by dropping every vector initializer,
# which would silently undo hif-backend#36 on the vector path.
if(NOT verilog_content MATCHES "pre = 4'b0101")
    message(FATAL_ERROR
        "Output port 'pre' states an initial value of \"0101\" in the VHDL and it is not present in the "
        "regenerated Verilog: suppressing the implicit default must not suppress a stated one "
        "(hif-backend#36).\nFull content:\n${verilog_content}")
endif()

# --- The property all of the above is really about: the output builds. -------
execute_process(
    COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${WORK_DIR}/vector_unknown_default.vvp ${OUTPUT_VERILOG}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog does not compile (exit code ${result}) -- this is the reported symptom of "
        "hif-backend#55.\n${compile_output}\nFull content:\n${verilog_content}")
endif()

# --- And reparses, since the frontend aborted on it too. ---------------------
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o vector_unknown_default_reparsed ${OUTPUT_VERILOG}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog failed to reparse (exit code ${result}).\nFull content:\n${verilog_content}")
endif()

message(STATUS "vector_unknown_default test passed.")
