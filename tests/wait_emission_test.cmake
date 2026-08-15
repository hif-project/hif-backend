# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#42): `wait` nodes were emitted without
#           their keyword or timescale, splicing their conditions and delays
#           into the next statement.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE VVP_EXECUTABLE
                 FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o wait_emission ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to parse cleanly.")
endif()

set(HIF_FILE ${WORK_DIR}/wait_emission.hif.xml)
if(NOT EXISTS ${HIF_FILE})
    message(FATAL_ERROR "Expected HIF file not produced: ${HIF_FILE}")
endif()

file(READ ${HIF_FILE} hif_content)
if(NOT hif_content MATCHES "<WAIT")
    message(FATAL_ERROR
        "The frontend produced no Wait node, so this fixture is exercising the frontend, not hif2verilog's wait emission.\n")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${HIF_FILE} -D ${WORK_DIR}/verilog_out
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "hif2verilog failed with exit code ${result} (expected 0)")
endif()

set(OUTPUT_VERILOG ${WORK_DIR}/verilog_out/wait_emission.v)
if(NOT EXISTS ${OUTPUT_VERILOG})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${OUTPUT_VERILOG}")
endif()

file(READ ${OUTPUT_VERILOG} verilog_content)

if(NOT verilog_content MATCHES "wait[ \t]*\\(")
    message(FATAL_ERROR
        "Regenerated Verilog contains no condition wait: `wait` keyword was not emitted (hif-backend#42).\n"
        "Full content:\n${verilog_content}")
endif()

if(NOT verilog_content MATCHES "@[ \t]*\\(")
    message(FATAL_ERROR
        "Regenerated Verilog contains no event control: `@` keyword was not emitted (hif-backend#42).\n"
        "Full content:\n${verilog_content}")
endif()

if(NOT verilog_content MATCHES "#[ \t]*4")
    message(FATAL_ERROR
        "Regenerated Verilog contains no delay statement: `#` keyword was not emitted (hif-backend#42).\n"
        "Full content:\n${verilog_content}")
endif()

# The design's only delay is the wait's, so before the fix timescale discovery 
# (which scanned only Assign delays) found nothing and emitted no directive.
if(NOT verilog_content MATCHES "`timescale")
    message(FATAL_ERROR
        "Regenerated Verilog contains no timescale directive: timescale discovery missed wait delays (hif-backend#42).\n"
        "Full content:\n${verilog_content}")
endif()

# The three identifiers the defect invented by concatenation on this exact
# fixture: `en`+`o`, `clk`+`o`, and the unexpanded timescale-unit constant the
# delay is carried as. Matched as whole words, so an unrelated identifier that
# merely contains one of them does not read as a regression.
foreach(spliced eno clko hif_verilog_timescale_unit)
    if(verilog_content MATCHES "(^|[^A-Za-z0-9_])${spliced}([^A-Za-z0-9_]|$)")
        message(FATAL_ERROR
            "Regenerated Verilog contains the identifier `${spliced}`, which exists in no source: it is a wait's "
            "operand concatenated onto the statement that followed it (hif-backend#42).\n"
            "Full content:\n${verilog_content}")
    endif()
endforeach()

file(MAKE_DIRECTORY ${WORK_DIR}/reparse)
file(COPY ${OUTPUT_VERILOG} DESTINATION ${WORK_DIR}/reparse/)
execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o wait_emission_reparsed wait_emission.v
    WORKING_DIRECTORY ${WORK_DIR}/reparse
    RESULT_VARIABLE result
    OUTPUT_VARIABLE reparse_output
    ERROR_VARIABLE reparse_output
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated Verilog does not parse, which is what the spliced identifiers caused (hif-backend#42).\n"
        "Exit code ${result}:\n${reparse_output}\n"
        "Full content:\n${verilog_content}")
endif()

function(simulate label source out_trace)
    set(vvp_image ${WORK_DIR}/${label}.vvp)
    execute_process(
        COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${vvp_image} ${source} ${TESTBENCH}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE compile_output
        ERROR_VARIABLE compile_output
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "iverilog failed to compile ${label} (${source}), exit code ${result}:\n${compile_output}")
    endif()

    execute_process(
        COMMAND ${VVP_EXECUTABLE} ${vvp_image}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE trace
        ERROR_VARIABLE run_errors
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "vvp failed to run ${label}, exit code ${result}:\n${run_errors}")
    endif()

    string(REPLACE "\r\n" "\n" trace "${trace}")
    string(REGEX REPLACE "\n?[^\n]*\\$finish[^\n]*" "" trace "${trace}")
    string(STRIP "${trace}" trace)
    set(${out_trace} "${trace}" PARENT_SCOPE)
endfunction()

simulate(original ${FIXTURE} original_trace)
simulate(regenerated ${OUTPUT_VERILOG} regenerated_trace)

if(NOT original_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated Verilog does not reproduce the source's behaviour: a dropped or mis-emitted wait changes when the process advances (hif-backend#42).\n"
        "--- original trace ---\n${original_trace}\n"
        "--- regenerated trace ---\n${regenerated_trace}\n"
        "--- regenerated source ---\n${verilog_content}")
endif()

message(STATUS "wait_emission test passed.")
