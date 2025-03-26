/// @file VerilogPrinter.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2verilog/VerilogPrinter.hpp"

#include <hif/semantics/declarationUtils.hpp>

#include <utility>

// Namespace hifsuite
using namespace hif;

VerilogPrinter::VerilogPrinter(hif::backends::IndentedStream *stream)
    : _sem(hif::semantics::VHDLSemantics::getInstance())
    , _stream(stream)
{
    // ntd
}

VerilogPrinter::~VerilogPrinter()
{
    // ntd
}

// Check
auto VerilogPrinter::visitAggregate(Aggregate &o) -> int
{
    GuideVisitor::visitAggregate(o);
    return 0;
}

// Check
auto VerilogPrinter::visitAggregateAlt(AggregateAlt &o) -> int
{
    GuideVisitor::visitAggregateAlt(o);
    return 0;
}

// Check
auto VerilogPrinter::visitAlias(Alias &o) -> int
{
    GuideVisitor::visitAlias(o);
    return 0;
}

// Check
auto VerilogPrinter::visitArray(Array &o) -> int
{
    (*_stream) << "array( ";
    // Print Span
    o.getSpan()->acceptVisitor(*this);
    (*_stream) << ") ";
    // Print Type
    (*_stream) << "of ";
    o.getType()->acceptVisitor(*this);
    return 0;
}

auto VerilogPrinter::visitAssign(Assign &o) -> int
{
    o.getLeftHandSide()->acceptVisitor(*this);

    auto dd = hif::semantics::getDeclaration(o.getLeftHandSide(), _sem);

    // "<=" in Verilog is called non-blocking assignment which brings a whole lot of difference than "=" which is called
    // as blocking assignment because of scheduling events in any vendor based simulators.
    //
    // It is Recommended to use non-blocking assignment for sequential logic and blocking assignment for combinational
    // logic, only then it infers correct hardware logic during synthesis.
    //
    // Non-blocking statements in sequential block will infer flip flop in actual hardware.
    //
    // Always remember do not mix blocking and non-blocking in any sequential or combinational block.
    if (dynamic_cast<Variable *>(dd) != nullptr) {
        (*_stream) << " = ";
    } else {
        (*_stream) << " <= ";
    }
    o.getRightHandSide()->acceptVisitor(*this);
    (*_stream) << ";\n";
    return 0;
}

// Check std_logic and std_ulogic
auto VerilogPrinter::visitBit(Bit &o) -> int
{
    GuideVisitor::visitBit(o);
    return 0;
}

auto VerilogPrinter::visitBitValue(BitValue &o) -> int
{
    auto value = this->getValue(&o);
    if (!value.empty()) {
        (*_stream) << value;
    }
    return 0;
}

auto VerilogPrinter::visitBitvector(Bitvector &o) -> int
{
    // Get the bitvector span.
    auto span        = o.getSpan();
    // Get the left bound.
    auto left_bound  = dynamic_cast<hif::IntValue *>(span->getLeftBound());
    // Get the right bound.
    auto right_bound = dynamic_cast<hif::IntValue *>(span->getRightBound());
    // If both bounds are valid, print the bitvector.
    if (left_bound && right_bound) {
        (*_stream) << "[" << left_bound->getValue() << ":" << right_bound->getValue() << "]";
        return 0;
    } else {
        return GuideVisitor::visitBitvector(o);
    }
}

auto VerilogPrinter::visitBitvectorValue(BitvectorValue &o) -> int
{
    auto value = this->getValue(&o);
    if (!value.empty()) {
        (*_stream) << value;
    }
    return 0;
}

auto VerilogPrinter::visitBool(Bool &o) -> int { return GuideVisitor::visitBool(o); }

auto VerilogPrinter::visitBoolValue(BoolValue &o) -> int { return GuideVisitor::visitBoolValue(o); }

auto VerilogPrinter::visitBreak(Break &o) -> int { return GuideVisitor::visitBreak(o); }

auto VerilogPrinter::visitCast(Cast &o) -> int
{
#if 0
    // Get the value being casted.
    auto value      = o.getValue();
    // Cast the value to an identifier.
    auto identifier = dynamic_cast<Identifier *>(value);
    // Check if the value is an identifier.
    if (!identifier) {
        return GuideVisitor::visitCast(o);
    }
    // Get the declaration of the value being casted.
    auto declaration = hif::semantics::getDeclaration(value, _sem);
    if (!declaration) {
        return GuideVisitor::visitCast(o);
    }
    // Check if the declaration is a Bitvector.
    hif::Bitvector *bitvector = nullptr;
    // Check if the declaration is a port, and extract the type.
    if (auto port = dynamic_cast<hif::Port *>(declaration)) {
        bitvector = dynamic_cast<hif::Bitvector *>(port->getType());
    } else {
        bitvector = dynamic_cast<hif::Bitvector *>(declaration);
    }
    if (!bitvector) {
        return GuideVisitor::visitCast(o);
    }
    // Get the type the value is begin casted to.
    auto type           = o.getType();
    // Check if the type is a Bitvector.
    auto bitvector_cast = dynamic_cast<hif::Bitvector *>(type);
    if (!bitvector_cast) {
        return GuideVisitor::visitCast(o);
    }
    // If the length of the destination bitvector is greater than the source bitvector, then we need to pad the
    // identifier with zeros.
    bool need_padding   = false;
    // Now first we need to
    auto int_to_width   = dynamic_cast<hif::IntValue *>(bitvector_cast->getSpan()->getLeftBound());
    auto int_from_width = dynamic_cast<hif::IntValue *>(bitvector->getSpan()->getLeftBound());
    if (int_to_width && int_from_width) {
        auto to_width   = int_to_width->getValue();
        auto from_width = int_from_width->getValue();
        if (to_width > from_width) {
            // Notify that we need padding.
            need_padding    = true;
            // Compute the difference in width.
            auto difference = to_width - from_width;
            // Build the padding.
            (*_stream) << "{";
            (*_stream) << difference << "'b";
            for (int i = 0; i < difference; ++i) {
                (*_stream) << "0";
            }
            (*_stream) << ", ";
        }
    }
    // Print the identifier name.
    (*_stream) << identifier->getName();
    if (need_padding) {
        (*_stream) << "}";
    }
    return 0;
#else
    (*_stream) << this->getValue(o.getValue());
    return 0;
    // return GuideVisitor::visitCast(o);
#endif
}

auto VerilogPrinter::visitChar(Char &o) -> int { return GuideVisitor::visitChar(o); }

auto VerilogPrinter::visitCharValue(CharValue &o) -> int { return GuideVisitor::visitCharValue(o); }

auto VerilogPrinter::visitConst(Const &o) -> int { return GuideVisitor::visitConst(o); }

auto VerilogPrinter::visitContents(Contents &o) -> int { return hif::GuideVisitor::visitContents(o); }

auto VerilogPrinter::visitContinue(Continue &o) -> int { return GuideVisitor::visitContinue(o); }

auto VerilogPrinter::visitDesignUnit(DesignUnit &o) -> int
{
    messageAssert(o.views.size() == 1, "Not supported more than one view", &o, _sem);

    // Get the name of the design unit.
    auto name    = o.getName();
    // Get the current view.
    auto view    = o.views.front();
    // Get the view entity.
    auto entity  = view->getEntity();
    // Get the view content.
    auto content = view->getContents();

    // Print the module header.
    (*_stream) << "module " << name;
    entity->acceptVisitor(*this);
    (*_stream) << "\n";
    _stream->indent();

    // ========================================================================
    // PRINT VARIABLE DECLARATIONS
    // ========================================================================

    // Keep track if the view has variables.
    bool has_variables = false;

    // Print the list of template parameters.
    this->printList(view->templateParameters, ";", true, true);
    has_variables = has_variables || !view->templateParameters.empty();

    // Print the list of view declarations.
    this->printList(view->declarations, ";", true, true);
    has_variables = has_variables || !view->templateParameters.empty();

    // Print the list of content declarations.
    this->printList(content->declarations, ";", true, true);
    has_variables = has_variables || !view->templateParameters.empty();

    // Print the list of state table declarations.
    for (auto stateTable : content->stateTables) {
        this->printList(stateTable->declarations, ";", true, true);
        has_variables = has_variables || !stateTable->declarations.empty();
    }

    if (has_variables) {
        (*_stream) << "\n";
    }

    // ========================================================================
    // PRINT FUNCTION DECLARATIONS
    // ========================================================================

    bool has_functions = false;

    has_functions |= this->printFunctions(view->declarations);
    has_functions |= this->printFunctions(content->declarations);
    for (auto stateTable : content->stateTables) {
        has_functions |= this->printFunctions(stateTable->declarations);
    }

    if (has_functions) {
        (*_stream) << "\n";
    }

    // ========================================================================
    // PRINT THE REST
    // ========================================================================

    for (auto instance : content->instances) {
        instance->acceptVisitor(*this);
    }

    for (auto generate : content->generates) {
        generate->acceptVisitor(*this);
    }

    // Visit the view contents.
    for (auto stateTable : content->stateTables) {
        stateTable->acceptVisitor(*this);
    }

    _stream->unindent();
    (*_stream) << "endmodule\n";
    return 0;
}

auto VerilogPrinter::visitEnum(Enum &o) -> int { return hif::GuideVisitor::visitEnum(o); }

auto VerilogPrinter::visitEnumValue(EnumValue &o) -> int
{
    (*_stream) << o.getName();
    return 0;
}

auto VerilogPrinter::visitEvent(Event &o) -> int { return GuideVisitor::visitEvent(o); }

auto VerilogPrinter::visitExpression(Expression &o) -> int
{
    // Set use of parenthesis for expression operands.
    bool needOp1Paren =
        ((dynamic_cast<ConstValue *>(o.getValue1()) == nullptr) &&
         (dynamic_cast<Identifier *>(o.getValue1()) == nullptr) &&
         (dynamic_cast<PPAssign *>(o.getValue1()) == nullptr) &&
         (dynamic_cast<FunctionCall *>(o.getValue1()) == nullptr) && (dynamic_cast<Cast *>(o.getValue1()) == nullptr) &&
         (dynamic_cast<Member *>(o.getValue1()) == nullptr));

    bool needOp2Paren =
        ((dynamic_cast<ConstValue *>(o.getValue2()) == nullptr) &&
         (dynamic_cast<Identifier *>(o.getValue2()) == nullptr) &&
         (dynamic_cast<PPAssign *>(o.getValue2()) == nullptr) &&
         (dynamic_cast<FunctionCall *>(o.getValue2()) == nullptr) && (dynamic_cast<Cast *>(o.getValue2()) == nullptr) &&
         (dynamic_cast<Member *>(o.getValue2()) == nullptr));

    // Check the existence of 2nd operand. If it is present the expression
    // is binary (i.e., Op1 op Op2) otherwise it is unary (i.e., op Op1)

    // If binary expression, print Op1. Unless pow (recursive call).
    if (o.getValue2() != nullptr) {
        if (needOp1Paren) {
            (*_stream) << "(";
        }
        o.getValue1()->acceptVisitor(*this);
        if (needOp1Paren) {
            (*_stream) << ")";
        }
        (*_stream) << " ";
    }

    // Print operator.
    switch (o.getOperator()) {
    // Logical (boolean) operators
    case op_not:
        (*_stream) << "!";
        break;
    case op_or:
        (*_stream) << "||";
        break;
    case op_xor:
        (*_stream) << "^";
        break;
    case op_and:
        (*_stream) << "&&";
        break;

        // Binary (bitwise) operators
    case op_bnot:
        (*_stream) << "~";
        break;
    case op_bor:
        (*_stream) << "|";
        break;
    case op_bxor:
        (*_stream) << "^";
        break;
    case op_band:
        (*_stream) << "&";
        break;
    case op_sll:
        (*_stream) << "<<";
        break;
    case op_srl:
        (*_stream) << ">>";
        break;
    case op_rol:
        (*_stream) << "rol"; // TODO
        break;
    case op_ror:
        (*_stream) << "ror"; // TODO
        break;

        // Concatenation operator
    case op_concat:
        (*_stream) << "{ }";
        break;

        // Equality operators
    case op_eq:
    case op_case_eq:
        (*_stream) << "=";
        break;
    case op_neq:
    case op_case_neq:
        (*_stream) << "!=";
        break;

        // Relational operators
    case op_lt:
        (*_stream) << "<";
        break;
    case op_le:
        (*_stream) << "<=";
        break;
    case op_gt:
        (*_stream) << ">";
        break;
    case op_ge:
        (*_stream) << ">=";
        break;

        // Arithmetic operators
    case op_plus:
        (*_stream) << "+";
        break;
    case op_minus:
        (*_stream) << "-";
        break;
    case op_mult:
        (*_stream) << "*";
        break;
    case op_div:
        (*_stream) << "/";
        break;
    case op_rem:
        (*_stream) << "rem"; // TODO
        break;
    case op_pow:
        (*_stream) << "**";
        break;
    case op_abs:
        (*_stream) << "abs"; // TODO
        break;
    case op_ref:
        break; // TODO
    case op_deref:
        break; // TODO
    case op_sla:
        (*_stream) << "sla"; // TODO
        break;
    case op_sra:
        (*_stream) << "sra"; // TODO
        break;
    case op_mod:
        (*_stream) << "%";
        break;
    case op_andrd:
    case op_orrd:
    case op_xorrd:
    case op_assign:
    case op_conv:
    case op_bind:
    case op_log:
    case op_reverse:
    case op_size:
    case op_none:
    default:
        messageError("This operator should be managed in refinement steps.", &o, _sem);
    }
    (*_stream) << " ";

    // If binary expression, print Op2.
    if (o.getValue2() != nullptr) {
        if (needOp2Paren) {
            (*_stream) << "(";
        }
        o.getValue2()->acceptVisitor(*this);
        if (needOp2Paren) {
            (*_stream) << ")";
        }
    }
    // If unary expression, print Op1.
    else {
        if (needOp1Paren) {
            (*_stream) << "(";
        }
        o.getValue1()->acceptVisitor(*this);
        if (needOp1Paren) {
            (*_stream) << ")";
        }
    }
    return 0;
}

auto VerilogPrinter::visitFunctionCall(FunctionCall &o) -> int
{
    (*_stream) << o.getName() << "(";
    for (std::size_t i = 0; i < o.parameterAssigns.size(); ++i) {
        o.parameterAssigns.at(i)->acceptVisitor(*this);
        if (i < o.parameterAssigns.size() - 1) {
            (*_stream) << ", ";
        }
    }
    (*_stream) << ")";
    return 0;
    // return hif::GuideVisitor::visitFunctionCall(o);
}

auto VerilogPrinter::visitField(Field &o) -> int { return GuideVisitor::visitField(o); }

auto VerilogPrinter::visitFieldReference(FieldReference &o) -> int { return GuideVisitor::visitFieldReference(o); }

auto VerilogPrinter::visitFile(File &o) -> int { return GuideVisitor::visitFile(o); }

auto VerilogPrinter::visitFor(For &o) -> int
{
    // Print the for loop.
    (*_stream) << "for (";
    for (std::size_t i = 0; i < o.initValues.size(); ++i) {
        // Cast the init value to an assignment.
        auto assign = dynamic_cast<Assign *>(o.initValues.at(i));
        if (assign) {
            assign->getLeftHandSide()->acceptVisitor(*this);
            (*_stream) << " = ";
            assign->getRightHandSide()->acceptVisitor(*this);
        }
        if (i < o.initValues.size() - 1) {
            (*_stream) << ", ";
        }
    }
    (*_stream) << "; ";
    if (o.getCondition()) {
        (*_stream) << this->getValue(o.getCondition());
    }
    (*_stream) << "; ";
    for (std::size_t i = 0; i < o.stepActions.size(); ++i) {
        // Cast the init value to an assignment.
        auto assign = dynamic_cast<Assign *>(o.stepActions.at(i));
        if (assign) {
            assign->getLeftHandSide()->acceptVisitor(*this);
            (*_stream) << " = ";
            assign->getRightHandSide()->acceptVisitor(*this);
        }
        if (i < o.stepActions.size() - 1) {
            (*_stream) << ", ";
        }
    }
    (*_stream) << " ) begin ";
    if (!o.getName().empty()) {
        (*_stream) << ": " << o.getName();
    }
    (*_stream) << "\n";

    _stream->indent();

    for (auto for_action : o.forActions) {
        for_action->acceptVisitor(*this);
    }

    _stream->unindent();
    (*_stream) << "end\n";
    return 0;
}

auto VerilogPrinter::visitForGenerate(ForGenerate &o) -> int { return GuideVisitor::visitForGenerate(o); }

auto VerilogPrinter::visitFunction(Function &o) -> int
{
    // Get the return type of the function.
    auto return_type = o.getType();
    // Get the state table.
    auto state_table = o.getStateTable();

    (*_stream) << "function";
    if (return_type) {
        (*_stream) << " ";
        return_type->acceptVisitor(*this);
        (*_stream) << " ";
    }
    (*_stream) << o.getName() << ";\n";

    _stream->indent();

    // ========================================================================
    // RENAME THE RETURN VARIABLE
    // ========================================================================

    hif::Identifier return_variable_id(o.getName() + "_return");
    // Get the declaration of the return variable.
    hif::Declaration *return_variable = hif::semantics::getDeclaration(&return_variable_id, _sem, {&o});
    if (!return_variable) {
        return_variable = hif::semantics::getDeclaration(&return_variable_id, _sem, {state_table});
    }
    // If we found the return variable, we can rename it.
    if (return_variable) {
        // Get all the references to the return variable.
        hif::semantics::ReferencesSet list;
        hif::semantics::getReferences(return_variable, list, _sem, &o);
        // Set the name of the return variable.
        for (auto entity : list) {
            if (auto named_object = dynamic_cast<hif::features::INamedObject *>(entity)) {
                named_object->setName(o.getName());
            }
        }
    } else {
        std::cerr << "Cannot find the return variable" << std::endl;
    }

    // ========================================================================
    // PRINT VARIABLE DECLARATIONS
    // ========================================================================

    bool has_variables = false;

    // Print only the declarations that are not the return variable.
    for (auto parameter : o.parameters) {
        if (parameter->getName() == (o.getName() + "_return")) {
            continue;
        }
        (*_stream) << this->getDeclaration(parameter) << ";\n";
        has_variables = true;
    }
    for (auto templateParameter : o.templateParameters) {
        if (templateParameter->getName() == (o.getName() + "_return")) {
            continue;
        }
        (*_stream) << this->getDeclaration(templateParameter) << ";\n";
        has_variables = true;
    }
    for (auto declaration : state_table->declarations) {
        if (declaration->getName() == (o.getName() + "_return")) {
            continue;
        }
        (*_stream) << this->getDeclaration(declaration) << ";\n";
        has_variables = true;
    }

    if (has_variables) {
        (*_stream) << "\n";
    }

    // ========================================================================
    // PRINT FUNCTION CONTENT
    // ========================================================================

    (*_stream) << "begin\n";
    _stream->indent();

    for (auto state : state_table->states) {
        state->acceptVisitor(*this);
    }

    _stream->unindent();
    (*_stream) << "end\n";
    _stream->unindent();
    (*_stream) << "endfunction\n";

    return 0;
    // return GuideVisitor::visitFunction(o);
}

auto VerilogPrinter::visitGlobalAction(GlobalAction &o) -> int { return GuideVisitor::visitGlobalAction(o); }

auto VerilogPrinter::visitEntity(Entity &o) -> int
{
    // port_direction data_type [ port_size ] port_name, port_name, ...;
    if (!o.ports.empty()) {
        (*_stream) << "(" << '\n';
        // Increase indentation.
        _stream->indent();
        // Iterate the ports and print them.
        this->printList(o.ports, ",", true, false);
        // Decrese the indentation.
        _stream->unindent();
        (*_stream) << ");";
    }
    return 0;
}

auto VerilogPrinter::visitIdentifier(Identifier &o) -> int
{
    (*_stream) << o.getName();
    return 0;
}

auto VerilogPrinter::visitIf(If &o) -> int { return GuideVisitor::visitIf(o); }

auto VerilogPrinter::visitIfAlt(IfAlt &o) -> int { return GuideVisitor::visitIfAlt(o); }

auto VerilogPrinter::visitIfGenerate(IfGenerate &o) -> int { return GuideVisitor::visitIfGenerate(o); }

auto VerilogPrinter::visitInstance(Instance &o) -> int
{
    if (auto view_reference = dynamic_cast<ViewReference *>(o.getReferencedType())) {
        (*_stream) << view_reference->getDesignUnit() << " ";
        (*_stream) << o.getName() << "(" << '\n';
        // Increase indentation.
        _stream->indent();
        // Iterate the ports and print them.
        this->printList(o.portAssigns, ",", true, false);
        // Decrese the indentation.
        _stream->unindent();
        (*_stream) << ");\n\n";
    }
    return 0;
}

auto VerilogPrinter::visitInt(Int &o) -> int { return hif::GuideVisitor::visitInt(o); }

auto VerilogPrinter::visitIntValue(IntValue &o) -> int { return hif::GuideVisitor::visitIntValue(o); }

auto VerilogPrinter::visitLibraryDef(LibraryDef &o) -> int { return hif::GuideVisitor::visitLibraryDef(o); }

auto VerilogPrinter::visitLibrary(Library &o) -> int { return hif::GuideVisitor::visitLibrary(o); }

auto VerilogPrinter::visitMember(Member &o) -> int
{
    (*_stream) << this->getValue(&o);
    return 0;
}

auto VerilogPrinter::visitNull(Null &o) -> int { return hif::GuideVisitor::visitNull(o); }

auto VerilogPrinter::visitTransition(Transition &o) -> int { return hif::GuideVisitor::visitTransition(o); }

auto VerilogPrinter::visitParameterAssign(ParameterAssign &o) -> int
{
    return hif::GuideVisitor::visitParameterAssign(o);
}

auto VerilogPrinter::visitParameter(Parameter &o) -> int { return hif::GuideVisitor::visitParameter(o); }

auto VerilogPrinter::visitProcedureCall(ProcedureCall &o) -> int { return hif::GuideVisitor::visitProcedureCall(o); }

auto VerilogPrinter::visitPointer(Pointer &o) -> int { return hif::GuideVisitor::visitPointer(o); }

auto VerilogPrinter::visitPortAssign(PortAssign &o) -> int { return hif::GuideVisitor::visitPortAssign(o); }

auto VerilogPrinter::visitPort(Port &o) -> int { return hif::GuideVisitor::visitPort(o); }

auto VerilogPrinter::visitProcedure(Procedure &o) -> int { return hif::GuideVisitor::visitProcedure(o); }

auto VerilogPrinter::visitRange(Range &o) -> int { return hif::GuideVisitor::visitRange(o); }

auto VerilogPrinter::visitReal(Real &o) -> int { return hif::GuideVisitor::visitReal(o); }

auto VerilogPrinter::visitRealValue(RealValue &o) -> int { return hif::GuideVisitor::visitRealValue(o); }

auto VerilogPrinter::visitRecord(Record &o) -> int { return hif::GuideVisitor::visitRecord(o); }

auto VerilogPrinter::visitRecordValue(RecordValue &o) -> int { return hif::GuideVisitor::visitRecordValue(o); }

auto VerilogPrinter::visitRecordValueAlt(RecordValueAlt &o) -> int { return hif::GuideVisitor::visitRecordValueAlt(o); }

auto VerilogPrinter::visitReference(Reference &o) -> int { return hif::GuideVisitor::visitReference(o); }

auto VerilogPrinter::visitReturn(Return &o) -> int
{
    (void)o;
    // Skip return statements.
    return 0;
    // return hif::GuideVisitor::visitReturn(o);
}

auto VerilogPrinter::visitSignal(Signal &o) -> int { return hif::GuideVisitor::visitSignal(o); }

auto VerilogPrinter::visitSigned(Signed &o) -> int { return hif::GuideVisitor::visitSigned(o); }

auto VerilogPrinter::visitSlice(Slice &o) -> int
{
    (*_stream) << this->getValue(&o);
    return 0;
}

auto VerilogPrinter::visitState(State &o) -> int { return hif::GuideVisitor::visitState(o); }

auto VerilogPrinter::visitString(String &o) -> int { return hif::GuideVisitor::visitString(o); }

auto VerilogPrinter::visitStateTable(StateTable &o) -> int
{
    /// @brief The list of states of the statetable.
    //      BList<State> states;
    /// @brief List of edges.
    //      BList<Transition> edges;

    if (o.getFlavour() == hif::ProcessFlavour::pf_analog) {
        (*_stream) << "analog begin\n";
        _stream->indent();

        for (auto state : o.states) {
            state->acceptVisitor(*this);
        }

        _stream->unindent();
        (*_stream) << "\n";
        (*_stream) << "end\n";
    } else {
        (*_stream) << "always";
        if (!o.sensitivity.empty()) {
            (*_stream) << " @( ";
            this->printList(o.sensitivity, ",", false, false);
            (*_stream) << " )";
        } else if (!o.sensitivityPos.empty()) {
            (*_stream) << " @( posedge ";
            this->printList(o.sensitivityPos, ",", false, false);
            (*_stream) << " )";
        } else if (!o.sensitivityNeg.empty()) {
            (*_stream) << " @( negedge ";
            this->printList(o.sensitivityNeg, ",", false, false);
            (*_stream) << " )";
        }
        (*_stream) << " begin" << '\n';
        _stream->indent();

        for (auto state : o.states) {
            state->acceptVisitor(*this);
        }

        _stream->unindent();
        (*_stream) << "\n";
        (*_stream) << "end\n";
    }
    return 0;
}

auto VerilogPrinter::visitSystem(System &o) -> int { return hif::GuideVisitor::visitSystem(o); }

auto VerilogPrinter::visitSwitchAlt(SwitchAlt &o) -> int { return GuideVisitor::visitSwitchAlt(o); }

auto VerilogPrinter::visitSwitch(Switch &o) -> int
{

    (*_stream) << "case ( " << this->getValue(o.getCondition()) << " )\n";
    _stream->indent();
    for (hif::SwitchAlt *alternative : o.alts) {
        for (hif::Value *condition : alternative->conditions) {
            (*_stream) << this->getValue(condition) << " ";
        }
        (*_stream) << ": begin\n";
        _stream->indent();
        for (hif::Action *action : alternative->actions) {
            action->acceptVisitor(*this);
        }
        _stream->unindent();
        (*_stream) << "end\n";
    }
    if (!o.defaults.empty()) {
        (*_stream) << "default" << ": begin\n";
        _stream->indent();
        for (hif::Action *action : o.defaults) {
            action->acceptVisitor(*this);
        }
        _stream->unindent();
        (*_stream) << "end\n";
    }
    _stream->unindent();
    (*_stream) << "endcase\n";
    return 0;
}

auto VerilogPrinter::visitStringValue(StringValue &o) -> int { return GuideVisitor::visitStringValue(o); }

auto VerilogPrinter::visitTime(Time &o) -> int { return hif::GuideVisitor::visitTime(o); }

auto VerilogPrinter::visitTimeValue(TimeValue &o) -> int { return hif::GuideVisitor::visitTimeValue(o); }

auto VerilogPrinter::visitTypeDef(TypeDef &o) -> int { return GuideVisitor::visitTypeDef(o); }

auto VerilogPrinter::visitTypeReference(TypeReference &o) -> int { return GuideVisitor::visitTypeReference(o); }

auto VerilogPrinter::visitTypeTPAssign(TypeTPAssign &o) -> int { return hif::GuideVisitor::visitTypeTPAssign(o); }

auto VerilogPrinter::visitTypeTP(TypeTP &o) -> int { return hif::GuideVisitor::visitTypeTP(o); }

auto VerilogPrinter::visitUnsigned(Unsigned &o) -> int { return hif::GuideVisitor::visitUnsigned(o); }

auto VerilogPrinter::visitValueStatement(ValueStatement &o) -> int { return GuideVisitor::visitValueStatement(o); }

auto VerilogPrinter::visitValueTPAssign(ValueTPAssign &o) -> int { return hif::GuideVisitor::visitValueTPAssign(o); }

auto VerilogPrinter::visitValueTP(ValueTP &o) -> int { return hif::GuideVisitor::visitValueTP(o); }

auto VerilogPrinter::visitVariable(Variable &o) -> int { return hif::GuideVisitor::visitVariable(o); }

auto VerilogPrinter::visitView(View &o) -> int { return hif::GuideVisitor::visitView(o); }

auto VerilogPrinter::visitViewReference(ViewReference &o) -> int { return GuideVisitor::visitViewReference(o); }

auto VerilogPrinter::visitWait(Wait &o) -> int { return GuideVisitor::visitWait(o); }

auto VerilogPrinter::visitWhen(When &o) -> int { return GuideVisitor::visitWhen(o); }

auto VerilogPrinter::visitWhenAlt(WhenAlt &o) -> int { return GuideVisitor::visitWhenAlt(o); }

auto VerilogPrinter::visitWhile(While &o) -> int { return hif::GuideVisitor::visitWhile(o); }

auto VerilogPrinter::visitWith(With &o) -> int { return GuideVisitor::visitWith(o); }

auto VerilogPrinter::visitWithAlt(WithAlt &o) -> int { return GuideVisitor::visitWithAlt(o); }

// ==============================================================================
// Private methods
// ==============================================================================

std::string VerilogPrinter::getDeclaration(hif::Declaration *declaration)
{
    std::stringstream ss;
    if (auto variable = dynamic_cast<hif::Variable *>(declaration)) {
        ss << "reg ";
        ss << this->getBitwidth(variable->getType());
        ss << variable->getName();
        auto value = this->getValue(variable->getValue());
        if (!value.empty()) {
            ss << " = " << value;
        }
    } else if (auto signal = dynamic_cast<hif::Signal *>(declaration)) {
        ss << "reg ";
        ss << this->getBitwidth(signal->getType());
        ss << signal->getName();
        auto value = this->getValue(signal->getValue());
        if (!value.empty()) {
            ss << " = " << value;
        }
    } else if (auto port = dynamic_cast<hif::Port *>(declaration)) {
        switch (port->getDirection()) {
        case PortDirection::dir_in:
            ss << "input wire ";
            break;
        case PortDirection::dir_out:
            ss << "output reg ";
            break;
        case PortDirection::dir_inout:
            ss << "inout ";
            break;
        default:
            messageError("Unexpected PortDirection", declaration, _sem);
        }
        ss << this->getBitwidth(port->getType());
        ss << port->getName();
    } else if (auto parameter = dynamic_cast<hif::Parameter *>(declaration)) {
        if (parameter->getDirection() == PortDirection::dir_in) {
            ss << "input ";
            ss << this->getBitwidth(parameter->getType());
            ss << parameter->getName();
        }
    }
    // else {
    //     messageError("Unexpected Declaration", declaration, _sem);
    // }
    return ss.str();
}

std::string VerilogPrinter::getBitwidth(hif::Type *type)
{
    std::stringstream ss;
    unsigned long long bw = hif::semantics::typeGetSpanBitwidth(type, _sem);
    if (bw != 1) {
        ss << "[" << (bw - 1) << ":0] ";
    }
    return ss.str();
}

std::string VerilogPrinter::getValue(hif::Value *value)
{
    std::stringstream ss;
    if (auto bit_value = dynamic_cast<BitValue *>(value)) {
        if (bit_value->is01()) {
            ss << bit_value->toString();
        }
    }
    if (auto int_value = dynamic_cast<hif::IntValue *>(value)) {
        if (int_value->getValue()) {
            ss << int_value->getValue();
        }
    } else if (auto bitvector_value = dynamic_cast<BitvectorValue *>(value)) {
        if (bitvector_value->is01()) {
            // Cast the type to Bitvector to get the width.
            auto bitvector_type = dynamic_cast<hif::Bitvector *>(bitvector_value->getType());
            // If the type is valid, get the width.
            if (bitvector_type) {
                // get the span of the bitvector type.
                auto span = bitvector_type->getSpan();
                if (span) {
                    // Get the left bound of the span.
                    auto left_bound  = dynamic_cast<hif::IntValue *>(span->getLeftBound());
                    // Get the right bound of the span.
                    auto right_bound = dynamic_cast<hif::IntValue *>(span->getRightBound());
                    // If both bounds are valid, compute the width.
                    if (left_bound && right_bound) {
                        auto width = left_bound->getValue() - right_bound->getValue() + 1;
                        ss << width << "'b";
                    }
                }
                ss << bitvector_value->getValue();
            }
        }
    } else if (auto identifier = dynamic_cast<hif::Identifier *>(value)) {
        if (!identifier->getName().empty()) {
            ss << identifier->getName();
        }
    } else if (auto member = dynamic_cast<hif::Member *>(value)) {
        // Get the member prefix.
        auto prefix = member->getPrefix();
        // Get the index.
        auto index  = member->getIndex();
        // If both the prefix and the index are valid, build the string.
        if (prefix && index) {
            // Visit the prefix.
            ss << this->getValue(prefix);
            // Add the access operator.
            ss << "[";
            // Visit the index.
            ss << this->getValue(index);
            // Close the access operator.
            ss << "]";
        }
    } else if (auto slice = dynamic_cast<hif::Slice *>(value)) {
        // Get the prefix.
        auto prefix = slice->getPrefix();
        // Get span.
        auto span   = slice->getSpan();
        // If both the prefix and the span are valid, build the string.
        if (prefix && span) {
            // Visit the prefix.
            ss << this->getValue(prefix);
            // Add the access operator.
            ss << "[";
            // Visit the span.
            ss << this->getValue(span);
            // Close the access operator.
            ss << "]";
        }
    } else if (auto range = dynamic_cast<hif::Range *>(value)) {
        // Get the left bound.
        auto left_bound  = range->getLeftBound();
        // Get the right bound.
        auto right_bound = range->getRightBound();
        // If both the left and right bounds are valid, build the string.
        if (left_bound && right_bound) {
            // Visit the left bound.
            ss << this->getValue(left_bound);
            // Add the range operator.
            ss << ":";
            // Visit the right bound.
            ss << this->getValue(right_bound);
        }
    } else if (auto expression = dynamic_cast<hif::Expression *>(value)) {
        // Visit the expression.
        expression->acceptVisitor(*this);
    } else if (auto cast = dynamic_cast<hif::Cast *>(value)) {
        ss << this->getValue(cast->getValue());
    }
    return ss.str();
}

std::string VerilogPrinter::getPortAssign(hif::PortAssign *port_assign)
{
    std::stringstream ss;
    if (auto identifier = dynamic_cast<hif::Identifier *>(port_assign->getValue())) {
        ss << "." << port_assign->getName() << "(" << identifier->getName() << ")";
    } else {
        ss << "." << port_assign->getName() << "(";
        port_assign->getValue()->acceptVisitor(*this);
        ss << ")";
    }
    return ss.str();
}

void VerilogPrinter::printList(
    const hif::BList<hif::Object> &list,
    const std::string &separator,
    bool print_new_line,
    bool print_last_separator)
{
    if (list.empty()) {
        return;
    }
    if (!_stream) {
        return;
    }
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (auto declaration = dynamic_cast<hif::DataDeclaration *>(list.at(i))) {
            // Get the declaration string.
            auto declaration_string = this->getDeclaration(declaration);
            // If the string is not empty, print it.
            if (!declaration_string.empty()) {
                (*_stream) << declaration_string;
            } else {
                continue;
            }
        } else if (auto value = dynamic_cast<hif::Value *>(list.at(i))) {
            value->acceptVisitor(*this);
        } else if (dynamic_cast<hif::Function *>(list.at(i))) {
            // Skip functions, print them at the beginning after the variables
            // declaration in the module.
            continue;
        } else if (auto port_assign = dynamic_cast<hif::PortAssign *>(list.at(i))) {
            // Get the port assign as a string.
            auto port_assign_string = this->getPortAssign(port_assign);
            // If the string is not empty, print it.
            if (!port_assign_string.empty()) {
                (*_stream) << port_assign_string;
            } else {
                continue;
            }
        }
        if (i < (list.size() - 1)) {
            if (!separator.empty()) {
                (*_stream) << separator;
            }
            if (print_new_line) {
                (*_stream) << "\n";
            } else {
                (*_stream) << " ";
            }
        } else {
            if (!separator.empty() && print_last_separator) {
                (*_stream) << separator;
            }
            if (print_new_line) {
                (*_stream) << "\n";
            }
        }
    }
}

bool VerilogPrinter::printFunctions(const hif::BList<hif::Object> &list)
{
    bool has_functions = false;
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (auto function = dynamic_cast<hif::Function *>(list.at(i))) {
            function->acceptVisitor(*this);
            has_functions = true;
        }
    }
    return has_functions;
}
