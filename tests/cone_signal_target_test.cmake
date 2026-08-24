# -----------------------------------------------------------------------------
# @brief  : Guards the invariant the hif-backend#16 fix rests on.
#
#           visitProcedureCall expands a cone's body at its call site. The
#           reads that follow the call - the reads of the cone's target that
#           motivated it - observe the value just computed only because the
#           target is assigned with blocking "=". A cone target that arrived as
#           something emitted with "<=" would send those reads silently back to
#           the previous value: the staleness of #16 again, this time inside a
#           single process.
#
#           The set of blocking-assigned targets is isBlockingAssignmentTarget
#           in VerilogPrinter.cpp: a Variable, or a Parameter with direction out
#           or inout (hif-backend#70). A Signal is in neither group, which is
#           what this fixture exercises.
#
#           hif-frontend guarantees Variable cone targets today
#           (refineToVariables shadows a target that must stay a signal into
#           a "_sig_var" Variable, driven from its own process), so this
#           cannot be produced from Verilog input. The fixture is therefore a
#           hand-built HIF: mini.hif.xml with the cone target declared as a
#           SIGNAL instead of a VARIABLE, everything else byte-identical.
#
#           Requirement: hif2verilog rejects it loudly. Emitting plausible
#           but stale-reading Verilog is the failure being prevented, so
#           "succeeds and produces output" is the wrong outcome here.
#
#           The positive control is sensitivity_equivalence, which exercises
#           the ordinary Variable-target path end to end.
# @author : Enrico Fraccaroli
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

if(result EQUAL 0)
    message(FATAL_ERROR
        "hif2verilog accepted a cone whose target is not assigned with blocking '='. It would "
        "be emitted with non-blocking '<=', leaving the reads after the inlined call observing "
        "a stale value (hif-backend#16). Expected a non-zero exit.\nTool output:\n${tool_output}")
endif()

# The diagnostic has to name the problem. A crash, or an assert from somewhere
# unrelated, would also exit non-zero without telling anyone what broke.
if(NOT tool_output MATCHES "Inlined cone assigns")
    message(FATAL_ERROR
        "hif2verilog rejected the fixture, but not with the cone-target diagnostic - so the "
        "non-zero exit may be incidental rather than this invariant firing.\nTool output:\n${tool_output}")
endif()

message(STATUS "cone_signal_target test passed.")
