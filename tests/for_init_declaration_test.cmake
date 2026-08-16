# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#47): VerilogPrinter::visitFor printed a
#           For's initValues and stepActions but never its initDeclarations, so
#           a loop that declares its own index regenerated with an undeclared,
#           uninitialised index and an empty init clause:
#
#               for (; index_0 <= 3; index_0 = index_0 + 1 ) begin
#                   @( posedge clk );
#               end
#
#           at exit code 0. verilog2hif lowers `repeat (n) @( ... )` into
#           exactly that shape, so every repeat in a Verilog source came back
#           broken. iverilog cannot bind index_0 and the design does not
#           compile.
#
#           Two separate things were missing, and a fix could supply one
#           without the other: the *declaration*, and the index's starting
#           value, which HIF carries on the declaration rather than in the loop
#           header. An index that is declared but never initialised compiles
#           and runs, and releases at the wrong edge.
#
#           This fixture is Verilog, so unlike the VHDL-sourced tests the
#           source itself can be simulated. The check is therefore a trace
#           comparison against the original rather than a hand-computed oracle:
#           the regenerated design has to behave exactly as the source does,
#           and the samples straddle the third clock edge so that releasing one
#           edge early or late shows up.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VERILOG2HIF_EXECUTABLE HIF2VERILOG_EXECUTABLE IVERILOG_EXECUTABLE VVP_EXECUTABLE FIXTURE TESTBENCH WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

# Simulate a Verilog source together with the shared testbench and return its
# trace. Any non-zero exit is fatal - a design that does not compile or run
# cannot be compared.
function(simulate label source out_trace)
    set(vvp_image ${WORK_DIR}/${label}.vvp)
    execute_process(
        COMMAND ${IVERILOG_EXECUTABLE} -g2005 -o ${vvp_image} ${source} ${TESTBENCH}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE compile_output
        ERROR_VARIABLE compile_output
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "${label} did not compile (exit ${result}):\n${compile_output}")
    endif()

    execute_process(
        COMMAND ${VVP_EXECUTABLE} ${vvp_image}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE trace
        ERROR_VARIABLE trace_err
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${label} simulation failed (exit ${result}):\n${trace}${trace_err}")
    endif()

    set(${out_trace} "${trace}" PARENT_SCOPE)
endfunction()

# --- Reference: the source design ---------------------------------------

simulate(source ${FIXTURE} source_trace)

# The comparison is only worth anything if the reference actually exercised the
# wait. Asserted rather than assumed, so that a testbench change that stopped
# reaching the third edge would be reported as itself instead of quietly making
# both traces trivially equal.
string(FIND "${source_trace}" "t30_three_edges o=0100" found_at)
if(found_at EQUAL -1)
    message(FATAL_ERROR
        "The source design did not write o by the third clock edge, so this test no longer "
        "exercises hif-backend#47.\nSource trace:\n${source_trace}")
endif()

# --- Round trip: Verilog -> HIF -> Verilog ------------------------------

execute_process(
    COMMAND ${VERILOG2HIF_EXECUTABLE} -o for_init_declaration ${FIXTURE}
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Setup step (verilog2hif) failed with exit code ${result} -- this fixture is expected to translate cleanly.")
endif()

set(hif_file ${WORK_DIR}/for_init_declaration.hif.xml)
if(NOT EXISTS ${hif_file})
    message(FATAL_ERROR "Expected HIF file not produced: ${hif_file}")
endif()

# The defect is only reachable when the loop actually declares its own index,
# which is what `repeat` lowers to. If a future frontend change stopped doing
# that, this test would still pass while covering nothing.
file(READ ${hif_file} hif_content)
string(FIND "${hif_content}" "<INITDECLARATIONS>" found_at)
if(found_at EQUAL -1)
    message(FATAL_ERROR
        "The HIF has no For::initDeclarations, so this test no longer exercises hif-backend#47.")
endif()

execute_process(
    COMMAND ${HIF2VERILOG_EXECUTABLE} ${hif_file} -D ${WORK_DIR}/out
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
    ERROR_VARIABLE stderr_text
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "hif2verilog failed with exit code ${result} (hif-backend#47).\nOutput was:\n${stderr_text}")
endif()

set(generated ${WORK_DIR}/out/for_init_declaration.v)
if(NOT EXISTS ${generated})
    message(FATAL_ERROR "Expected regenerated Verilog not produced: ${generated}")
endif()

file(READ ${generated} verilog_content)

# --- The index is declared, and initialised -----------------------------

# Checked separately from the trace because they fail differently: a missing
# declaration does not compile at all, while a missing initialisation compiles
# and releases at the wrong edge. Reporting which one happened is worth more
# than a bare trace mismatch.
string(REGEX MATCH "for[ \t]*\\([ \t]*;" empty_init "${verilog_content}")
if(empty_init)
    message(FATAL_ERROR
        "Regenerated Verilog has a for loop with an empty init clause, so the index keeps whatever "
        "value it had (hif-backend#47).\nFull content:\n${verilog_content}")
endif()

string(REGEX MATCH "(integer|reg[^;]*)[ \t]+index_[0-9]+[ \t]*;" index_decl "${verilog_content}")
if(NOT index_decl)
    message(FATAL_ERROR
        "Regenerated Verilog never declares the loop index (hif-backend#47).\n"
        "Full content:\n${verilog_content}")
endif()

# --- Behaviour matches the source ---------------------------------------

simulate(regenerated ${generated} regenerated_trace)

if(NOT source_trace STREQUAL regenerated_trace)
    message(FATAL_ERROR
        "Regenerated design does not behave like its source.\n"
        "Source trace:\n${source_trace}\n"
        "Regenerated trace:\n${regenerated_trace}\n"
        "Regenerated Verilog:\n${verilog_content}")
endif()

message(STATUS "for_init_declaration test passed.")
