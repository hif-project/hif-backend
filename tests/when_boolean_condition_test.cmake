# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#54): lowering a Verilog ternary for VHDL
#           produced an If whose condition kept the Verilog one-bit type. VHDL
#           accepts only boolean there, so hif-core's checker rejected the tree:
#
#             CHECK HIF - ERROR! condition's type is not allowed.
#               - Raised by hif-core/src/semantics/checkHif.cpp:2575
#             Wrong HIF description after PreRefine_misc
#             Assertion `false' failed.
#
#           Exit 134, no VHDL produced. Conditional assignment is ordinary RTL,
#           so this made Verilog -> HIF -> VHDL fail for most real designs.
#
#           What discriminates here is the exit code, and only in a build with
#           assertions live. Measured, rather than assumed: a Release build of
#           the pre-fix toolchain emits VHDL *byte-identical* to the fixed one,
#           because a later refinement pass renders a boolean cast of a bit as
#           `x = '1'` regardless. So the tree was only transiently invalid, at
#           the checkStep boundary after PreRefine_misc -- which is precisely
#           what that check exists to catch, and why the fix belongs at the
#           point the If is built rather than in the checker.
#
#           The consequence for this test: the structural assertions below do
#           NOT fail against a Release pre-fix build. They are here to document
#           the required condition form and to catch a regression in the later
#           pass, not as the discriminator. The discriminator is that hif2vhdl
#           exits 134 before the fix under the build CI actually uses (ci.yml
#           passes no CMAKE_BUILD_TYPE, so assertions are live).
#
#           No VHDL analyzer is available locally or in CI (the workflow installs
#           iverilog only), so the strongest available check that the output is
#           real VHDL is that vhdl2hif accepts it again -- the same standard the
#           verilog_to_vhdl test uses.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VHDL_EXECUTABLE VHDL2HIF_EXECUTABLE FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o when_boolean_condition ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to translate cleanly.")
endif()

set(hif_file ${WORK_DIR}/when_boolean_condition.hif.xml)
if(NOT EXISTS ${hif_file})
    message(FATAL_ERROR "Expected HIF file not produced: ${hif_file}")
endif()

# The ternaries have to still be Whens for this to be about When lowering. If a
# future frontend change folds them into something else, every check below would
# pass while covering nothing.
file(READ ${hif_file} hif_content)
if(NOT hif_content MATCHES "<WHEN")
    message(FATAL_ERROR
        "verilog2hif produced no When for the conditional assignments, so this test no longer exercises "
        "hif-backend#54.")
endif()

execute_process(
    COMMAND ${HIF2VHDL_EXECUTABLE} ${hif_file} -D ${WORK_DIR}/out
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
    ERROR_VARIABLE stderr_text
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "hif2vhdl failed on a design containing a conditional assignment, exit code ${result} "
        "(hif-backend#54).\nOutput was:\n${stderr_text}")
endif()

set(generated ${WORK_DIR}/out/src/when_boolean_condition.vhd)
if(NOT EXISTS ${generated})
    message(FATAL_ERROR "Expected regenerated VHDL not produced: ${generated}")
endif()
file(SIZE ${generated} size)
if(size EQUAL 0)
    message(FATAL_ERROR "hif2vhdl exited 0 but left a zero-byte file: ${generated}")
endif()

file(READ ${generated} vhdl_content)

# --- A bit-typed condition becomes an explicit comparison. -------------------
# Documents the required form. Note this holds even for a Release pre-fix build,
# where a later pass produces it anyway -- see the header. It guards the later
# pass, not the fix.
if(NOT vhdl_content MATCHES "IF sel = '1' THEN")
    message(FATAL_ERROR
        "The ternary's bit-typed condition was not coerced to boolean; VHDL does not allow a bit there "
        "(hif-backend#54).\nFull content:\n${vhdl_content}")
endif()

# --- A condition that is already boolean is left alone. ----------------------
# Verilog `==` yields one bit, but the coercion of a comparison has to collapse
# to the plain VHDL comparison rather than stacking a redundant test on top.
if(NOT vhdl_content MATCHES "IF a = b THEN")
    message(FATAL_ERROR
        "The comparison-conditioned ternary did not emit a plain boolean comparison.\n"
        "Full content:\n${vhdl_content}")
endif()

# --- No illegal conversion-call spelling. ------------------------------------
# A literal Cast to boolean would print as `boolean(sel)`, which VHDL has no
# conversion for. The refinement is expected to render it as a comparison.
if(vhdl_content MATCHES "boolean *\\(")
    message(FATAL_ERROR
        "Regenerated VHDL contains a `boolean(...)` conversion, which VHDL does not define for std_logic.\n"
        "Full content:\n${vhdl_content}")
endif()

# --- Both lowering paths are covered. ----------------------------------------
# The concurrent ternaries become when_functions; the in-process one becomes an
# If in place. A fix applied to only one call site would miss the other.
if(NOT vhdl_content MATCHES "when_function")
    message(FATAL_ERROR
        "The concurrent conditional assignments were not lowered into when_functions.\n"
        "Full content:\n${vhdl_content}")
endif()

# --- The output is real VHDL, not plausible text. ----------------------------
execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o when_boolean_condition_reparsed ${generated}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated VHDL failed to reparse (exit code ${result}).\nFull content:\n${vhdl_content}")
endif()

message(STATUS "when_boolean_condition test passed.")
