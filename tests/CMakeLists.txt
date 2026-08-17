# -----------------------------------------------------------------------------
# @brief  : Tests cmake file.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

# This regression needs verilog2hif (a sibling repo's tool, not a build
# dependency of this one) to produce its input HIF. Look on PATH and, since
# this project is normally checked out alongside its siblings, in
# hif-frontend's build directory too.
find_program(
    VERILOG2HIF_EXECUTABLE
    NAMES verilog2hif
    HINTS ${CMAKE_SOURCE_DIR}/../hif-frontend/build
)
find_program(
    VHDL2HIF_EXECUTABLE
    NAMES vhdl2hif
    HINTS ${CMAKE_SOURCE_DIR}/../hif-frontend/build
)

# Looked up here rather than beside the first test that needs one, because
# both the Verilog-sourced and the VHDL-sourced groups below gate on these.
find_program(IVERILOG_EXECUTABLE NAMES iverilog)
find_program(VVP_EXECUTABLE NAMES vvp)

if(VERILOG2HIF_EXECUTABLE)
    add_test(
        NAME unresolved_parameter
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/unresolved_parameter.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/unresolved_parameter_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/unresolved_parameter_test.cmake
    )
    # Regression: hif2verilog crashes printing op_andrd/op_orrd (the
    # reduction-AND/OR operators used internally to implement ~& and ~|).
    add_test(
        NAME reduction_operators
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/reduction_operators.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/reduction_operators_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/reduction_operators_test.cmake
    )
    # Regression: hif2verilog renders an unresolved top-level parametric
    # port width as a huge unsigned-wraparound literal, and never prints
    # the parameter's own declaration at all.
    add_test(
        NAME parametric_port_width
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/parametric_port_width.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/parametric_port_width_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/parametric_port_width_test.cmake
    )
    # Regression: hif2verilog silently dropped Procedure declarations (the
    # "cone functions" hif-frontend synthesizes for primitive gate
    # instances and flattened combinational submodule instances) instead
    # of printing their driving logic.
    add_test(
        NAME gate_primitives
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/gate_primitives.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/gate_primitives_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/gate_primitives_test.cmake
    )
    # Same root cause as gate_primitives, from plain module instantiation.
    add_test(
        NAME module_instantiation
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/module_instantiation.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/module_instantiation_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/module_instantiation_test.cmake
    )
    # Regression: concatenation printed as "a { } b" (an infix operator
    # token) instead of "{a, b}".
    add_test(
        NAME concat_emission
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/concat_emission.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/concat_emission_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/concat_emission_test.cmake
    )
    # Regression: `localparam` declarations (HIF Const) were silently
    # dropped from regenerated Verilog.
    add_test(
        NAME localparam_emission
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/localparam_emission.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/localparam_emission_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/localparam_emission_test.cmake
    )
    # Regression (hif-backend#23): assigning to a bit-select aborted the
    # printer, leaving a zero-byte output file behind.
    add_test(
        NAME bitselect_target
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/bitselect_target.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/bitselect_target_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/bitselect_target_test.cmake
    )

    # Regression (hif-backend#18): slice/part-select emission produced
    # invalid Verilog - a part-select applied to an expression, and
    # parameter-expression bounds printed ahead of their identifier.
    add_test(
        NAME slice_emission
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/slice_emission.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/slice_emission_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/slice_emission_test.cmake
    )

    # Regression (hif-backend#19): a Verilog system function ($clog2) was
    # emitted under its internal, mangled name, which no simulator accepts
    # and the frontend cannot bind on the way back in.
    add_test(
        NAME system_function_round_trip
        COMMAND ${CMAKE_COMMAND}
            -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/system_function_round_trip.v
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/system_function_round_trip_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/system_function_round_trip_test.cmake
    )

    # Regression (hif-backend#16): regenerated Verilog was not behaviorally
    # equivalent to its source - cone Procedures were hoisted into their own
    # always blocks, leaving the processes that read their targets not
    # sensitive to them. This one needs a simulator: the broken output is
    # syntactically perfect, so no text or reparse check can see it.
    if(IVERILOG_EXECUTABLE AND VVP_EXECUTABLE)
        add_test(
            NAME sensitivity_equivalence
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/sensitivity_equivalence.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/sensitivity_equivalence_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/sensitivity_equivalence_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/sensitivity_equivalence_test.cmake
        )
        # Regression (hif-backend#21): only one of a process's three
        # sensitivity lists was emitted, and the posedge/negedge keyword was
        # printed once per list rather than once per signal - regenerating
        # an asynchronous reset as a synchronous one. Also invisible to a
        # text or reparse check.
        add_test(
            NAME edge_sensitivity
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/edge_sensitivity.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/edge_sensitivity_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/edge_sensitivity_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/edge_sensitivity_test.cmake
        )
        # Regression (hif-backend#26): a structure-preserving round trip
        # regenerated the parent's connection nets as reg while they were
        # still bound to child instance output ports, which Verilog forbids -
        # so the regenerated hierarchy did not compile at all. Needs a
        # simulator for the same reason the two above do: the check that
        # matters is that the output is accepted and behaves identically.
        add_test(
            NAME preserved_hierarchy
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE_PARENT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/preserved_hierarchy.v
                -DFIXTURE_CHILD=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/preserved_hierarchy_child.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/preserved_hierarchy_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/preserved_hierarchy_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/preserved_hierarchy_test.cmake
        )
        # Regression (hif-backend#24): the delay on a delayed assignment was
        # dropped, so the regenerated design responded immediately where its
        # source waited. Invisible without simulating with timing: the output
        # parsed and reparsed cleanly either way.
        add_test(
            NAME assign_delay
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/assign_delay.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/assign_delay_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/assign_delay_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/assign_delay_test.cmake
        )
        # Regression (hif-backend#30): verilog2hif folds a constant continuous
        # assignment into the driven port's initial value, and that value was
        # printed nowhere - Verilog-2001 has no place for an initializer in an
        # ANSI port list - so the output regenerated with no driver at all.
        # Needs a simulator: the broken output compiles and reparses, and the
        # only symptom is that the output reads x.
        add_test(
            NAME port_initial_value
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/port_initial_value.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/port_initial_value_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/port_initial_value_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/port_initial_value_test.cmake
        )
        # Regression (hif-backend#29): system task calls ($display, $write)
        # were emitted as nothing at all, so a round trip discarded the whole
        # observable output of a design. Needs a simulator, because the
        # question is literally whether the regenerated design still prints.
        # The fixture also carries gate primitives, so the cone-inlining path
        # this shares with visitProcedureCall is exercised at the same time.
        add_test(
            NAME system_task_calls
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/system_task_calls.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/system_task_calls_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/system_task_calls_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/system_task_calls_test.cmake
        )
        # Regression (hif-backend#40): a process with no sensitivity list was
        # emitted as a bare `always begin ... end`, a zero-delay infinite loop
        # that Icarus rejects at elaboration - so every design containing an
        # `initial` block regenerated into something that does not compile,
        # silently. Needs a simulator twice over: the failure is an
        # elaboration error rather than a parse error, and the fix must be
        # shown not to reorder what the initial block computes.
        add_test(
            NAME initial_process
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/initial_process.v
                -DWAIT_FIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/initial_process_wait.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/initial_process_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/initial_process_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/initial_process_test.cmake
        )
        # Regression (hif-backend#42): `wait` nodes were emitted without their
        # keyword or timescale, splicing their conditions and delays into the
        # next statement.
        add_test(
            NAME wait_emission
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/wait_emission.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/wait_emission_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/wait_emission_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/wait_emission_test.cmake
        )
        # Regression (hif-backend#38): a user-defined Verilog task was inlined
        # as if it were a frontend-synthesized logic cone, which tripped the
        # hif-backend#16 assert and aborted with a zero-byte output file - so a
        # design containing an ordinary task could not be regenerated at all.
        # The fixture carries a task and a gate primitive's cone together,
        # because the fix is a distinction between the two rather than a change
        # to either. Needs a simulator: a task emitted with the wrong
        # assignment kind compiles and reads stale values.
        add_test(
            NAME user_task
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/user_task.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/user_task_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/user_task_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/user_task_test.cmake
        )
        # Regression (hif-backend#47): visitFor printed a For's initValues but
        # never its initDeclarations, so `repeat (n) @( ... )` - which
        # verilog2hif lowers into a loop that declares its own index -
        # regenerated with an undeclared, uninitialised index and an empty init
        # clause. Needs a simulator: the fixture is Verilog, so the source can
        # be simulated as a reference and the two traces compared, which also
        # catches an index that is declared but starts from the wrong value.
        add_test(
            NAME for_init_declaration
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/for_init_declaration.v
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/for_init_declaration_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/for_init_declaration_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/for_init_declaration_test.cmake
        )
    else()
        message(STATUS "iverilog/vvp not found; skipping sensitivity_equivalence/edge_sensitivity/preserved_hierarchy/assign_delay/for_init_declaration tests.")
    endif()
else()
    message(STATUS "verilog2hif not found; skipping unresolved_parameter/reduction_operators/parametric_port_width/gate_primitives/module_instantiation/concat_emission/localparam_emission/bitselect_target/slice_emission/system_function_round_trip tests.")
endif()

# The VHDL backend's own output. Needs vhdl2hif rather than verilog2hif, so it
# is gated separately - a checkout with one frontend tool but not the other is
# not a shape worth reasoning about, but the gate should still name what it
# wants.
if(VHDL2HIF_EXECUTABLE)
    # Regression (hif-backend#27): hif2vhdl left the design unit it emitted
    # last as a zero-byte file while exiting 0 - which, for a single-unit
    # design, was the whole output.
    add_test(
        NAME vhdl_output_flushed
        COMMAND ${CMAKE_COMMAND}
            -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
            -DHIF2VHDL_EXECUTABLE=$<TARGET_FILE:hif2vhdl>
            -DSINGLE_FIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/vhdl_single_unit.vhd
            -DHIERARCHY_FIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/vhdl_hierarchy.vhd
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/vhdl_output_flushed_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/vhdl_output_flushed_test.cmake
    )

    # Regression (hif-backend#46): a VHDL process ending in `wait;` regenerated
    # as a zero-delay `always begin ... end`. Needs a simulator: the stimulus
    # changes an input after both processes have suspended, and a process that
    # looped instead of stopping would pick that change up.
    if(IVERILOG_EXECUTABLE AND VVP_EXECUTABLE)
        add_test(
            NAME permanent_suspension
            COMMAND ${CMAKE_COMMAND}
                -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/permanent_suspension.vhd
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/permanent_suspension_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/permanent_suspension_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/permanent_suspension_test.cmake
        )
    else()
        message(STATUS "iverilog/vvp not found; skipping permanent_suspension test.")
    endif()

    # Regression (hif-backend#51): a VHDL clocked process regenerated as a call
    # to the undeclared hif_vhdl_event/hif_vhdl_rising_edge, because VHDL keeps
    # its clock edge in the body while Verilog keeps it in the sensitivity
    # list. Needs a simulator: a rebuild that produced `always @(clk, rst)`
    # instead of edges would still compile and still reparse, and only the
    # timing of the outputs tells a flip-flop from a level-sensitive process.
    if(IVERILOG_EXECUTABLE AND VVP_EXECUTABLE)
        add_test(
            NAME clocked_process
            COMMAND ${CMAKE_COMMAND}
                -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/clocked_process.vhd
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/clocked_process_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/clocked_process_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/clocked_process_test.cmake
        )
    else()
        message(STATUS "iverilog/vvp not found; skipping clocked_process test.")
    endif()

    # Regression (hif-backend#50): VerilogPrinter::getValue had no terminal
    # case, so a value kind absent from its dynamic_cast chain rendered as the
    # empty string. A FunctionCall took that path, so `if rising_edge(clk)`
    # regenerated as `if (  )` at exit code 0. Needs vhdl2hif only: the check
    # is that the condition survives into the output, and the result does not
    # yet compile for an unrelated reason (hif-backend#51).
    add_test(
        NAME condition_rendering
        COMMAND ${CMAKE_COMMAND}
            -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
            -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/condition_rendering.vhd
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/condition_rendering_work
            -P ${CMAKE_CURRENT_SOURCE_DIR}/condition_rendering_test.cmake
    )

    # Regression (hif-backend#48): hif2vhdl aborted with "Unsupported scope" on
    # every verilog2hif output, because When lowering descended into the
    # standard Verilog library and found a When in a SubProgram signature -
    # a scope that is neither a Function nor a Contents. Verilog->VHDL was
    # unavailable for every input. Needs verilog2hif to produce the input and
    # vhdl2hif to reparse the result; no simulator, since the failure was a
    # non-zero exit and an empty output directory rather than a wrong value.
    if(VERILOG2HIF_EXECUTABLE)
        add_test(
            NAME verilog_to_vhdl
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VHDL_EXECUTABLE=$<TARGET_FILE:hif2vhdl>
                -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/verilog_to_vhdl.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/verilog_to_vhdl_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/verilog_to_vhdl_test.cmake
        )
        # Regression (hif-backend#54): lowering a Verilog ternary for VHDL left
        # the If condition bit-typed, which VHDL does not allow, so hif-core's
        # checker rejected the tree and hif2vhdl aborted. Conditional assignment
        # is ordinary RTL, so this blocked Verilog->VHDL for most real designs.
        # No simulator: no VHDL analyzer is available locally or in CI, so the
        # gate is the exit code, the emitted condition form, and a reparse.
        add_test(
            NAME when_boolean_condition
            COMMAND ${CMAKE_COMMAND}
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DHIF2VHDL_EXECUTABLE=$<TARGET_FILE:hif2vhdl>
                -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/when_boolean_condition.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/when_boolean_condition_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/when_boolean_condition_test.cmake
        )
    else()
        message(STATUS "verilog2hif not found; skipping verilog_to_vhdl test.")
    endif()

    # Regression (hif-backend#32): a view's GlobalAction was never printed, so
    # every VHDL concurrent signal assignment was dropped and VHDL->Verilog
    # produced a module with the right ports and an empty body. Needs both
    # frontends (vhdl2hif to produce the input, verilog2hif to check the output
    # still reparses) and a simulator: the broken output compiles and reparses
    # perfectly, so only running it shows that it drives nothing.
    if(VERILOG2HIF_EXECUTABLE AND IVERILOG_EXECUTABLE AND VVP_EXECUTABLE)
        add_test(
            NAME global_action_emission
            COMMAND ${CMAKE_COMMAND}
                -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/global_action_emission.vhd
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/global_action_emission_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/global_action_emission_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/global_action_emission_test.cmake
        )
        # Regression (hif-backend#36): a VHDL output port's explicit initial
        # value was dropped when a process drove the port, so the regenerated
        # design read x until that process first wrote it. The other half of
        # the field hif-backend#30 covers: there the port has no other driver
        # and the value becomes a continuous assign, here it is a reg and the
        # value becomes an `initial` assignment. Needs a simulator because the
        # broken output compiles and reparses and the only symptom is a value.
        add_test(
            NAME port_initial_process
            COMMAND ${CMAKE_COMMAND}
                -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/port_initial_process.vhd
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/port_initial_process_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/port_initial_process_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/port_initial_process_test.cmake
        )
        # Regression (hif-backend#57): a user-defined function was emitted with
        # an empty body, so every call returned x and every condition built on
        # one selected the wrong branch. Needs a simulator: the broken output
        # compiles and reparses, and the only symptom is the value.
        add_test(
            NAME user_function_body
            COMMAND ${CMAKE_COMMAND}
                -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DVVP_EXECUTABLE=${VVP_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/user_function_body.vhd
                -DTESTBENCH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/user_function_body_tb.v
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/user_function_body_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/user_function_body_test.cmake
        )
    else()
        message(STATUS "verilog2hif/iverilog/vvp not all found; skipping global_action_emission test.")
    endif()

    # Regression (hif-backend#55): a VHDL vector output port with no stated
    # initial value regenerated as `initial vec = 4'buuuu;`, and `u` is not a
    # Verilog binary digit, so the output did not parse. The same rule as
    # hif-backend#36 - the frontend's implicit 'U' default is not a value the
    # source asked for - left unapplied on the vector path. Needs iverilog to
    # show the reported symptom, but no simulator: the defect is that the
    # output does not build, not that it computes the wrong thing.
    if(VERILOG2HIF_EXECUTABLE AND IVERILOG_EXECUTABLE)
        add_test(
            NAME vector_unknown_default
            COMMAND ${CMAKE_COMMAND}
                -DVHDL2HIF_EXECUTABLE=${VHDL2HIF_EXECUTABLE}
                -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
                -DVERILOG2HIF_EXECUTABLE=${VERILOG2HIF_EXECUTABLE}
                -DIVERILOG_EXECUTABLE=${IVERILOG_EXECUTABLE}
                -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/vector_unknown_default.vhd
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/vector_unknown_default_work
                -P ${CMAKE_CURRENT_SOURCE_DIR}/vector_unknown_default_test.cmake
        )
    else()
        message(STATUS "verilog2hif/iverilog not both found; skipping vector_unknown_default test.")
    endif()
else()
    message(STATUS "vhdl2hif not found; skipping vhdl_output_flushed/global_action_emission tests.")
endif()

# Guards the invariant the hif-backend#16 fix rests on: a cone target must be
# a Variable, so the inlined body is assigned with blocking "=" and the reads
# after the call see the value just computed. The fixture is a hand-built HIF
# because hif-frontend cannot currently produce this shape, which is also why
# this test needs no verilog2hif and runs regardless of it.
add_test(
    NAME cone_signal_target
    COMMAND ${CMAKE_COMMAND}
        -DHIF2VERILOG_EXECUTABLE=$<TARGET_FILE:hif2verilog>
        -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cone_signal_target.hif.xml
        -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/cone_signal_target_work
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cone_signal_target_test.cmake
)
