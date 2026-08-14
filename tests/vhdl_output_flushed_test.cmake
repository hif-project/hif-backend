# -----------------------------------------------------------------------------
# @brief  : Regression (hif-backend#27): hif2vhdl left the design unit it
#           emitted last as a zero-byte file, while exiting 0.
#
#           IndentedStream buffers and writes from its own destructor.
#           _initializeOutstream deletes the previous stream before opening the
#           next, so every unit but the last was written as a side effect of
#           starting the one after it - and the last one, never. Its file had
#           already been created, so nothing about the exit code or the presence
#           of the output said anything was wrong.
#
#           Two cases, because the reported shape hides how broad it was:
#
#             1. one entity, no hierarchy - the only output file was empty, so
#                hif2vhdl produced nothing at all for the design;
#             2. a hierarchy, parent written second - which is how it was
#                reported, the child looking fine and the parent lost.
#
#           A zero-byte file is the failure mode #23 was also about: it is not
#           obviously invalid to a consumer that only checks the exit code.
#           Emptiness is therefore asserted directly rather than inferred from
#           a later step failing.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

foreach(required VHDL2HIF_EXECUTABLE HIF2VHDL_EXECUTABLE SINGLE_FIXTURE HIERARCHY_FIXTURE WORK_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

# Translate a fixture to HIF and back to VHDL, then require every expected
# output file to exist and to be non-empty.
function(round_trip label fixture expected_units out_dir)
    execute_process(
        COMMAND ${VHDL2HIF_EXECUTABLE} -o ${label} ${fixture}
        WORKING_DIRECTORY ${WORK_DIR}
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Setup step (vhdl2hif) failed for ${label} with exit code ${result} -- this fixture is expected to parse cleanly.")
    endif()

    set(hif_file ${WORK_DIR}/${label}.hif.xml)
    if(NOT EXISTS ${hif_file})
        message(FATAL_ERROR "Expected HIF file not produced: ${hif_file}")
    endif()

    execute_process(
        COMMAND ${HIF2VHDL_EXECUTABLE} ${hif_file} -D ${WORK_DIR}/${label}_out
        WORKING_DIRECTORY ${WORK_DIR}
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "hif2vhdl failed for ${label} with exit code ${result} (expected 0)")
    endif()

    foreach(unit ${expected_units})
        set(generated ${WORK_DIR}/${label}_out/src/${unit}.vhd)
        if(NOT EXISTS ${generated})
            message(FATAL_ERROR "Expected regenerated VHDL not produced: ${generated}")
        endif()
        file(SIZE ${generated} size)
        if(size EQUAL 0)
            message(FATAL_ERROR
                "hif2vhdl exited 0 but left a zero-byte file for design unit '${unit}': ${generated} "
                "(hif-backend#27).")
        endif()
    endforeach()

    set(${out_dir} ${WORK_DIR}/${label}_out/src PARENT_SCOPE)
endfunction()

# --- Case 1: a single design unit ---------------------------------------

round_trip(vhdl_single_unit ${SINGLE_FIXTURE} "vhdl_single_unit" single_src)

file(READ ${single_src}/vhdl_single_unit.vhd single_content)
foreach(expected "ENTITY vhdl_single_unit" "ARCHITECTURE rtl")
    string(FIND "${single_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Regenerated VHDL is non-empty but missing '${expected}'.\nFull content:\n${single_content}")
    endif()
endforeach()

# --- Case 2: a hierarchy, parent emitted last ---------------------------

round_trip(vhdl_hierarchy ${HIERARCHY_FIXTURE} "vhdl_hierarchy_child;vhdl_hierarchy" hierarchy_src)

file(READ ${hierarchy_src}/vhdl_hierarchy.vhd parent_content)

# Non-empty is not enough for the parent: what was lost is the hierarchy, so
# the structure that carries it has to be there.
foreach(expected
    "ENTITY vhdl_hierarchy"
    "ARCHITECTURE rtl"
    "COMPONENT vhdl_hierarchy_child"
    "u_ha1"
    "u_ha2"
    "PORT MAP")
    string(FIND "${parent_content}" "${expected}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Regenerated parent unit is missing '${expected}', so the hierarchy did not survive.\n"
            "Full content:\n${parent_content}")
    endif()
endforeach()

# Both instantiations, not just one.
string(REGEX MATCHALL "PORT MAP" port_map_matches "${parent_content}")
list(LENGTH port_map_matches port_map_count)
if(port_map_count LESS 2)
    message(FATAL_ERROR
        "Regenerated parent unit has ${port_map_count} PORT MAP(s); expected 2.\nFull content:\n${parent_content}")
endif()

# No VHDL simulator is assumed here, so the strongest available check that the
# output is real VHDL rather than plausible text is that the frontend accepts
# it again.
execute_process(
    COMMAND ${VHDL2HIF_EXECUTABLE} -o vhdl_hierarchy_reparsed
            ${hierarchy_src}/vhdl_hierarchy_child.vhd ${hierarchy_src}/vhdl_hierarchy.vhd
    WORKING_DIRECTORY ${WORK_DIR}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Regenerated hierarchical VHDL failed to reparse (exit code ${result}).\nFull content:\n${parent_content}")
endif()

# And that the instances are still there afterwards: a parent that reparsed but
# had lost its port maps would otherwise pass.
file(READ ${WORK_DIR}/vhdl_hierarchy_reparsed.hif.xml reparsed_content)
string(REGEX MATCHALL "<INSTANCE" reparsed_instances "${reparsed_content}")
list(LENGTH reparsed_instances reparsed_instance_count)
if(reparsed_instance_count LESS 2)
    message(FATAL_ERROR
        "Reparsed HIF carries ${reparsed_instance_count} Instance node(s); expected at least 2.")
endif()

message(STATUS "vhdl_output_flushed test passed.")
