/// @file VerilogPrinter.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2verilog/VerilogPrinter.hpp"
#include "hif2verilog/PrintMethods.hpp"

#include <utility>

// Namespace hifsuite
using namespace hif;

VerilogPrinter::VerilogPrinter(hif::backends::IndentedStream *stream)
    : _sem(hif::semantics::VHDLSemantics::getInstance())
    , _stream(stream)
    , _currentDesignUnitName("")
    , _currentViewName("")
    , _currentView(nullptr)
    , _currentSystem(nullptr)
    , _currentContents(nullptr)
    , _currentEntity(nullptr)
    , _printedComponents()
    , _ams_enabled(false)
    , _isPrintComponents(false)
    , _isPrintWithCondition(false)
    , _isPrintingLibDefDecls(false)
    , _isSubProgramBody(false)
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
    *(_stream) << "array( ";
    // Print Span
    o.getSpan()->acceptVisitor(*this);
    *(_stream) << ") ";
    // Print Type
    *(_stream) << "of ";
    o.getType()->acceptVisitor(*this);
    return 0;
}

auto VerilogPrinter::visitAssign(Assign &o) -> int
{
    // handle With assign
    if (dynamic_cast<With *>(o.getRightHandSide()) != nullptr) {
        _isPrintWithCondition = true;
        o.getRightHandSide()->acceptVisitor(*this);
        _isPrintWithCondition = false;

        _stream->newLine();
        _stream->indent();
    }

    _stream->indent();

    o.getLeftHandSide()->acceptVisitor(*this);

    // Get the declaration type of the target
    Identifier *id = dynamic_cast<Identifier *>(hif::getTerminalPrefix(o.getLeftHandSide()));
    messageAssert(id != nullptr, "Unexpected target", &o, _sem);

    Declaration *dd = getDeclaration(id, _sem);

    // "<=" in Verilog is called non-blocking assignment which brings a whole lot of difference than "=" which is called
    // as blocking assignment because of scheduling events in any vendor based simulators.
    //
    // It is Recommended to use non-blocking assignment for sequential logic and blocking assignment for combinational
    // logic, only then it infers correct hardware logic during synthesis.
    //
    // Non-blocking statements in sequential block will infer flip flop in actual hardware.
    //
    // Always remember do not mix blocking and non-blocking in any sequential or combinational block.
    //if (_ams_enabled) {
    //    *(_stream) << " <+ ";
    //} // if the target is a variable put := otherwise put <=
    if (dynamic_cast<Variable *>(dd) != nullptr) {
        *(_stream) << " = ";
    } else {
        *(_stream) << " <= ";
    }

    o.getRightHandSide()->acceptVisitor(*this);
    _stream->unindent();

    return 0;
}

// Check std_logic and std_ulogic
auto VerilogPrinter::visitBit(Bit &o) -> int
{
    (void)o;
    *(_stream) << "wire";
    return 0;
}

// Verilog has four logic values, 0,1 Z, X. Values U, W, L, H, - are only in VHDL.
auto VerilogPrinter::visitBitValue(BitValue &o) -> int
{
    switch (o.getValue()) {
    case bit_zero:
        *(_stream) << "'0'";
        break;
    case bit_one:
        *(_stream) << "'1'";
        break;
    case bit_x:
        *(_stream) << "'X'";
        break;
    case bit_z:
        *(_stream) << "'Z'";
        break;
    case bit_u:
        *(_stream) << "'U'";
        break;
    case bit_w:
        *(_stream) << "'W'";
        break;
    case bit_l:
        *(_stream) << "'L'";
        break;
    case bit_h:
        *(_stream) << "'H'";
        break;
    case bit_dontcare:
        *(_stream) << "'-'";
        break;
    default: // unsupported
        messageError("Unexpected bit value", &o, _sem);
    }

    return 0;
}

auto VerilogPrinter::visitBitvector(Bitvector &o) -> int
{
    GuideVisitor::visitBitvector(o);
    return 0;
}

auto VerilogPrinter::visitBitvectorValue(BitvectorValue &o) -> int
{
    GuideVisitor::visitBitvectorValue(o);
    return 0;
}

auto VerilogPrinter::visitBool(Bool &o) -> int
{
    GuideVisitor::visitBool(o);
    return 0;
}

auto VerilogPrinter::visitBoolValue(BoolValue &o) -> int
{
    GuideVisitor::visitBoolValue(o);
    return 0;
}

auto VerilogPrinter::visitBreak(Break &o) -> int
{
    GuideVisitor::visitBreak(o);
    return 0;
}

auto VerilogPrinter::visitCast(Cast &o) -> int
{
    GuideVisitor::visitCast(o);
    return 0;
}

auto VerilogPrinter::visitChar(Char &o) -> int
{
    GuideVisitor::visitChar(o);
    return 0;
}

auto VerilogPrinter::visitCharValue(CharValue &o) -> int
{
    GuideVisitor::visitCharValue(o);
    return 0;
}

auto VerilogPrinter::visitConst(Const &o) -> int
{
    /*
    *(_stream) << "\tlocalparam " << o.getName();

    Value *v = o.getValue();
    if (v != nullptr)
    {
        // TODO How to get the starting value type? Because 'till now all costs seems to have a BVV
        if (dynamic_cast<BitvectorValue *>(v) != nullptr)
        {
            BitvectorValue *bvv = static_cast<BitvectorValue *>(v);
            Int *intType = new Int();
            Value *transfVal = hif::manipulation::transformValue(bvv, intType, _sem);
            if (dynamic_cast<IntValue *>(transfVal) != nullptr)
            {
                IntValue *iv = static_cast<IntValue *>(transfVal);
                *(_stream) << " = " << iv->getValue();
            } else {
             *(_stream) << " = " << bvv->getValue();
            }
        } else
            messageError("Unhandled localparam value", &o, _sem);
    }
    *(_stream) << ";\n";
    */
    GuideVisitor::visitConst(o);
    return 0;
}

auto VerilogPrinter::visitContents(Contents &o) -> int
{
    if (o.getName() == "Architecture") {
        this->processDeclarations(o);
        this->processStateTables(o);
    }
    return 0;
}

auto VerilogPrinter::visitContinue(Continue &o) -> int
{
    GuideVisitor::visitContinue(o);
    return 0;
}

auto VerilogPrinter::visitDesignUnit(DesignUnit &o) -> int
{
#if 0
    if (hif::languageIDToString(o.views.at(0)->getLanguageID()) == "AMS") {
        _ams_enabled = true;
        messageInfo("AMS enabled into the VerilogPrinter");
    }
#endif

    auto duName            = o.getName();
    _currentDesignUnitName = duName;
    *(_stream) << "module " << duName;
    messageAssert(o.views.size() == 1, "Not supported more than one view", &o, _sem);
    GuideVisitor::visitDesignUnit(o);
    _stream->newLine(1);
    *(_stream) << "endmodule // " << duName << "\n";
    return 0;
}

auto VerilogPrinter::visitEnum(Enum &o) -> int
{
    *(_stream) << "(";
    _printList(o.values, ',', false);
    *(_stream) << ")";

    return 0;
}

auto VerilogPrinter::visitEnumValue(EnumValue &o) -> int
{
    *(_stream) << o.getName();
    return 0;
}

auto VerilogPrinter::visitEvent(Event &o) -> int
{
    GuideVisitor::visitEvent(o);
    return 0;
}

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
            *(_stream) << "(";
        }
        o.getValue1()->acceptVisitor(*this);
        if (needOp1Paren) {
            *(_stream) << ")";
        }
        *(_stream) << " ";
    }

    // Print operator.
    switch (o.getOperator()) {
    // Logical (boolean) operators
    case op_not:
        *(_stream) << "!";
        break;
    case op_or:
        *(_stream) << "||";
        break;
    case op_xor:
        *(_stream) << "^";
        break;
    case op_and:
        *(_stream) << "&&";
        break;

        // Binary (bitwise) operators
    case op_bnot:
        *(_stream) << "~";
        break;
    case op_bor:
        *(_stream) << "|";
        break;
    case op_bxor:
        *(_stream) << "^";
        break;
    case op_band:
        *(_stream) << "&";
        break;
    case op_sll:
        *(_stream) << "<<";
        break;
    case op_srl:
        *(_stream) << ">>";
        break;
    case op_rol:
        *(_stream) << "rol"; // TODO
        break;
    case op_ror:
        *(_stream) << "ror"; // TODO
        break;

        // Concatenation operator
    case op_concat:
        *(_stream) << "{ }";
        break;

        // Equality operators
    case op_eq:
    case op_case_eq:
        *(_stream) << "=";
        break;
    case op_neq:
    case op_case_neq:
        *(_stream) << "!=";
        break;

        // Relational operators
    case op_lt:
        *(_stream) << "<";
        break;
    case op_le:
        *(_stream) << "<=";
        break;
    case op_gt:
        *(_stream) << ">";
        break;
    case op_ge:
        *(_stream) << ">=";
        break;

        // Arithmetic operators
    case op_plus:
        *(_stream) << "+";
        break;
    case op_minus:
        *(_stream) << "-";
        break;
    case op_mult:
        *(_stream) << "*";
        break;
    case op_div:
        *(_stream) << "/";
        break;
    case op_rem:
        *(_stream) << "rem"; // TODO
        break;
    case op_pow:
        *(_stream) << "**";
        break;
    case op_abs:
        *(_stream) << "abs"; // TODO
        break;
    case op_ref:
        break; // TODO
    case op_deref:
        break; // TODO
    case op_sla:
        *(_stream) << "sla"; // TODO
        break;
    case op_sra:
        *(_stream) << "sra"; // TODO
        break;
    case op_mod:
        *(_stream) << "%";
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
    *(_stream) << " ";

    // If binary expression, print Op2.
    if (o.getValue2() != nullptr) {
        if (needOp2Paren) {
            *(_stream) << "(";
        }
        o.getValue2()->acceptVisitor(*this);
        if (needOp2Paren) {
            *(_stream) << ")";
        }
    }
    // If unary expression, print Op1.
    else {
        if (needOp1Paren) {
            *(_stream) << "(";
        }
        o.getValue1()->acceptVisitor(*this);
        if (needOp1Paren) {
            *(_stream) << ")";
        }
    }
    return 0;
}

auto VerilogPrinter::visitFunctionCall(FunctionCall &o) -> int
{
    // handle attributes
    //_printValueInstance(o.getInstance());

    if (dynamic_cast<Alias *>(o.getParent()) == nullptr) {
        if (o.getName() == "hif_verilog_V") {
            *(_stream) << "V";
        } else if (o.getName() == "hif_verilog_I") {
            *(_stream) << "I";
        } else if (o.getName() == "hif_verilog_ddt") {
            *(_stream) << "ddt";
        } else if (o.getName() == "hif_verilog_Omega") {
            *(_stream) << "Omega";
        } else if (o.getName() == "hif_verilog_Tau") {
            *(_stream) << "Tau";
        } else {
            *(_stream) << o.getName();
        }

        // Print parameters
        if (!o.parameterAssigns.empty()) {
            *(_stream) << "(";
            _printList(o.parameterAssigns, ',', false);
            *(_stream) << ")";
        }
    } else {
        // Print parameters
        if (!o.parameterAssigns.empty()) {
            *(_stream) << "(";
            _printList(o.parameterAssigns, ',', false);
            *(_stream) << ")";
        }
    }

    return 0;
}

auto VerilogPrinter::visitField(Field &o) -> int
{
    GuideVisitor::visitField(o);
    return 0;
}

auto VerilogPrinter::visitFieldReference(FieldReference &o) -> int
{
    GuideVisitor::visitFieldReference(o);
    return 0;
}

auto VerilogPrinter::visitFile(File &o) -> int
{
    GuideVisitor::visitFile(o);
    return 0;
}

auto VerilogPrinter::visitFor(For &o) -> int
{
    GuideVisitor::visitFor(o);
    return 0;
}

auto VerilogPrinter::visitForGenerate(ForGenerate &o) -> int
{
    GuideVisitor::visitForGenerate(o);
    return 0;
}

auto VerilogPrinter::visitFunction(Function &o) -> int
{
    GuideVisitor::visitFunction(o);
    return 0;
}

auto VerilogPrinter::visitGlobalAction(GlobalAction &o) -> int
{
    GuideVisitor::visitGlobalAction(o);
    return 0;
}

auto VerilogPrinter::visitEntity(Entity &o) -> int
{
    _currentEntity = &o;

    // port_direction data_type [ port_size ] port_name, port_name, ...;
    if (!o.ports.empty()) {

        // Print Ports of the module.
        *(_stream) << "(" << '\n';

        // Increase indentation.
        _stream->indent();

        // Iterate the ports and print them.
        for (std::size_t idx = 0; idx < o.ports.size(); idx++) {
            // Get the port.
            auto port = o.ports.at(idx);
            // Print port direction.
            switch (port->getDirection()) {
            case PortDirection::dir_in:
                *(_stream) << "input ";
                break;
            case PortDirection::dir_out:
                *(_stream) << "output ";
                break;
            case PortDirection::dir_inout:
                *(_stream) << "inout ";
                break;
            default:
                messageError("Unexpected PortDirection", port, _sem);
            }
            // Print the type of the port.
            port->getType()->acceptVisitor(*this);
            // Print the name of the port.
            *(_stream) << " " << port->getName();
            // If the port is not the first one, print a comma.
            if (idx < (o.ports.size() - 1)) {
                *(_stream) << ", ";
            }
            *_stream << "\n";
        }

        _stream->unindent();
        *(_stream) << ");" << '\n';
    }

    // Print Template Parameters
    for (auto *tp : _currentView->templateParameters) {
        tp->acceptVisitor(*this);
    }

    *(_stream) << "\n";

    // unsigned long long bw = hif::semantics::typeGetSpanBitwidth(it3->getType(), _sem);
    //         if (bw != 1) {
    //             *(_stream) << "[" << (bw - 1) << ":0] ";
    //         }

    return 0;
}

auto VerilogPrinter::visitIdentifier(Identifier &o) -> int
{
    *(_stream) << o.getName();

    return 0;
}

auto VerilogPrinter::visitIf(If &o) -> int
{
    GuideVisitor::visitIf(o);
    return 0;
}

auto VerilogPrinter::visitIfAlt(IfAlt &o) -> int
{
    GuideVisitor::visitIfAlt(o);
    return 0;
}

// TODO
auto VerilogPrinter::visitIfGenerate(IfGenerate &o) -> int
{
    messageInfo("IfGenerate is not implemented yet.");

    GuideVisitor::visitIfGenerate(o);
    return 0;
}

auto VerilogPrinter::visitInstance(Instance &o) -> int
{
    auto *vr = dynamic_cast<ViewReference *>(o.getReferencedType());

    if (vr != nullptr) {
        _stream->indent();
        if (vr->getName() == "behav") {
            if (!o.portAssigns.empty()) {
                *(_stream) << vr->getDesignUnit() + " " + o.getName() + " (";
                // print ports
                _printList(o.portAssigns, ',', false);
                *(_stream) << ");\n";
            }
        }
        _stream->unindent();
    }

    return 0;
}

// TODO
auto VerilogPrinter::visitInt(Int &o) -> int
{
    *(_stream) << (o.isSigned() ? "integer" : "natural");

    return 0;
}

auto VerilogPrinter::visitIntValue(IntValue &o) -> int
{
    *(_stream) << o.getValue();
    return 0;
}

auto VerilogPrinter::visitLibraryDef(LibraryDef &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    std::string libraryDefName = o.getName();

    *(_stream) << "PACKAGE " << libraryDefName << " IS" << '\n' << '\n';
    _stream->indent();

    // Print LibraryDef content
    if (!o.declarations.empty()) {
        bool restore           = _isPrintingLibDefDecls;
        _isPrintingLibDefDecls = true;
        _printList(o.declarations, ';', true);
        *(_stream) << ";";
        _isPrintingLibDefDecls = restore;
    }

    _stream->unindent();
    *(_stream) << "\n\nEND " << libraryDefName << ";" << '\n';

    *(_stream) << "\n\nPACKAGE BODY " << libraryDefName << " IS" << '\n' << '\n';
    _stream->indent();

    // Custom printing to skip type defs.
    for (BList<Declaration>::iterator i = o.declarations.begin(); i != o.declarations.end(); ++i) {
        Declaration *d = *i;
        if (dynamic_cast<TypeDef *>(d) != nullptr) {
            continue;
        }
        d->acceptVisitor(*this);
        *(_stream) << ";";
        _stream->newLine();
    }

    _stream->unindent();
    *(_stream) << "\nEND " << libraryDefName << ";" << '\n' << std::flush;
    delete _stream;
    _stream = nullptr;
    return 0;
}

auto VerilogPrinter::visitLibrary(Library &o) -> int
{
    if (o.isStandard()) {
        return 0;
    }

    _printTypeInstance(o.getInstance());

    if (o.getInstance() == nullptr && !o.isStandard() && !o.isSystem()) {
        *(_stream) << "work.";
    }

    // TODO
    //*(_stream) << o.getName();

    return 0;
}

auto VerilogPrinter::visitMember(Member &o) -> int
{
    bool needParen = (dynamic_cast<Expression *>(o.getPrefix()) != nullptr);
    if (needParen) {
        *(_stream) << "(";
    }
    o.getPrefix()->acceptVisitor(*this);
    if (needParen) {
        *(_stream) << ")";
    }

    messageAssert(o.getIndex() != nullptr, "Unsupported member without index", &o, _sem);

    *(_stream) << "( ";
    o.getIndex()->acceptVisitor(*this);
    *(_stream) << " )";

    return 0;
}

auto VerilogPrinter::visitNull(Null & /*o*/) -> int
{
    *(_stream) << "nullptr";
    return 0;
}

auto VerilogPrinter::visitTransition(Transition &o) -> int
{
    messageError("Transition is not implemented yet.", &o, nullptr);
}

auto VerilogPrinter::visitParameterAssign(ParameterAssign &o) -> int
{
    if (o.getValue() != nullptr) {
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

auto VerilogPrinter::visitParameter(Parameter &o) -> int
{
    *(_stream) << o.getName();
    *(_stream) << ": ";

    if (o.getDirection() != dir_none) {
        _printPortDirection(o.getDirection());
        *(_stream) << " ";
    }

    o.getType()->acceptVisitor(*this);

    if (o.getValue() != nullptr) {
        *(_stream) << " := ";
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

auto VerilogPrinter::visitProcedureCall(ProcedureCall &o) -> int
{
    Declaration *decl = hif::semantics::getDeclaration(&o, _sem);
    if (_isSupportDeclaration(decl)) {
        return 0;
    }

    std::string name = o.getName();
    if (name == "hif_verilog_vams_indirect_contribution_statement") {
        assert(o.parameterAssigns.size() == 2);
        o.parameterAssigns.at(0)->acceptVisitor(*this);

        *(_stream) << ": ";

        auto *cast = dynamic_cast<Cast *>(o.parameterAssigns.at(1)->getValue());
        assert(cast);
        auto *expression = dynamic_cast<Expression *>(cast->getValue());
        assert(expression);
        expression->getValue2()->acceptVisitor(*this);
        *(_stream) << " == ";
        expression->getValue1()->acceptVisitor(*this);
    } else if (name == "hif_verilog_vams_contribution_statement") {
        //_stream->indent();
        assert(o.parameterAssigns.size() == 2);
        o.parameterAssigns.at(0)->acceptVisitor(*this);

        *(_stream) << " <+ ";

        auto *expression = dynamic_cast<Expression *>(o.parameterAssigns.at(1)->getValue());
        if (expression != nullptr) {
            expression->acceptVisitor(*this);
        }
        //expression->getValue2()->acceptVisitor(*this);
        //*(_stream) << " == ";
        //expression->getValue1()->acceptVisitor(*this);

        //_stream->unindent();
    } else {
        // handle attributes
        //_printValueInstance(o.getInstance());

        *(_stream) << o.getName();

        // Print parameters
        if (!o.parameterAssigns.empty()) {
            *(_stream) << "(";
            _printList(o.parameterAssigns, ',', false);
            *(_stream) << ")";
        }
    }
    return 0;
}

auto VerilogPrinter::visitPointer(Pointer &o) -> int
{
    *(_stream) << "access ";
    o.getType()->acceptVisitor(*this);

    return 0;
}

auto VerilogPrinter::visitPortAssign(PortAssign &o) -> int
{
    o.getValue()->acceptVisitor(*this);
    return 0;
}

auto VerilogPrinter::visitPort(Port &o) -> int
{
    return 0;
}

auto VerilogPrinter::visitProcedure(Procedure &o) -> int
{
    if (_isSupportDeclaration(&o)) {
        return 0;
    }

    *(_stream) << "PROCEDURE ";
    *(_stream) << o.getName();

    if (!o.parameters.empty()) {
        *(_stream) << " (";
        _printList(o.parameters, ';', false);
        *(_stream) << " )";
    }

    if (!_isPrintingLibDefDecls) {
        *(_stream) << " IS" << '\n';
        bool restore      = _isSubProgramBody;
        _isSubProgramBody = true;
        o.getStateTable()->acceptVisitor(*this);
        _isSubProgramBody = restore;
    }

    return 0;
}

auto VerilogPrinter::visitRange(Range &o) -> int
{
    *(_stream) << "[";
    o.getLeftBound()->acceptVisitor(*this);
    *(_stream) << ":";
    o.getRightBound()->acceptVisitor(*this);
    *(_stream) << "]";
    return 0;
}

auto VerilogPrinter::visitReal(Real & /*o*/) -> int
{
    *(_stream) << "real ";

    return 0;
}

auto VerilogPrinter::visitRealValue(RealValue &o) -> int
{
    *(_stream) << o.getValue();

    double whole   = NAN;
    double decimal = NAN;
    decimal        = std::modf(o.getValue(), &whole);
    if (_approximatelyEqual(decimal, 0.0, 0.001)) {
        //if (decimal == 0.0) {
        *(_stream) << ".0";
    }

    return 0;
}

auto VerilogPrinter::visitRecord(Record &o) -> int
{
    *(_stream) << "RECORD" << '\n';
    _stream->indent();
    _printList(o.fields, ';', true);
    _stream->unindent();
    *(_stream) << "END RECORD" << '\n';
    return 0;
}

auto VerilogPrinter::visitRecordValue(RecordValue &o) -> int
{
    *(_stream) << "( ";

    _printList(o.alts, ',', true);

    *(_stream) << " )";

    return 0;
}

auto VerilogPrinter::visitRecordValueAlt(RecordValueAlt &o) -> int
{
    *(_stream) << o.getName() << " <= ";
    o.getValue()->acceptVisitor(*this);

    return 0;
}

auto VerilogPrinter::visitReference(Reference &o) -> int
{
    messageError("Reference is not implemented yet.", &o, nullptr);
}

auto VerilogPrinter::visitReturn(Return &o) -> int
{
    *(_stream) << "return";
    if (o.getValue() != nullptr) {
        *(_stream) << " ";
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

auto VerilogPrinter::visitSignal(Signal &o) -> int
{
    // Check if AMS is enabled
    if (_ams_enabled) {
        _stream->indent();
        auto *currentView = dynamic_cast<ViewReference *>(o.getType());
        assert(currentView);
        // Check flavour because it could be a digital signal in an AMS design
        if (currentView->getName() == "ams_discipline") {
            std::string prefix("hif_verilog_");
            std::string currentType = currentView->getDesignUnit();

            if (currentType.compare(0, prefix.size(), prefix) == 0) {
                *(_stream) << currentType.substr(prefix.length()) + " " + o.getName() + ";";
            }
        }
    } else {
        // signal identifier : subtype_indication [ := expression ];
        // E.g.: signal name: integer range 7 downto 0;
        _stream->indent();
        *(_stream) << "signal " << o.getName() << "; ";

        //o.getType()->acceptVisitor( *this );

        if (o.getRange() != nullptr) {
            *(_stream) << " range ";
            o.getRange()->acceptVisitor(*this);
        }

        if (o.getValue() != nullptr) {
            *(_stream) << " := ";

            o.getValue()->acceptVisitor(*this);
        }
    }

    *(_stream) << "\n";
    _stream->unindent();

    return 0;
}

auto VerilogPrinter::visitSigned(Signed &o) -> int
{
    *(_stream) << "signed";

    if (dynamic_cast<Cast *>(o.getParent()) != nullptr) {
        return 0;
    }
    if (dynamic_cast<Function *>(o.getParent()) != nullptr) {
        return 0;
    }

    if (o.getSpan() != nullptr) {
        *(_stream) << "( ";
        o.getSpan()->acceptVisitor(*this);
        *(_stream) << " )";
    }

    return 0;
}

auto VerilogPrinter::visitSlice(Slice &o) -> int
{
    bool needParen = (dynamic_cast<Expression *>(o.getPrefix()) != nullptr);
    if (needParen) {
        *(_stream) << "(";
    }
    o.getPrefix()->acceptVisitor(*this);
    if (needParen) {
        *(_stream) << ")";
    }

    *(_stream) << "( ";
    o.getSpan()->acceptVisitor(*this);
    *(_stream) << " )";
    return 0;
}

auto VerilogPrinter::visitState(State &o) -> int
{
    if (!o.actions.empty()) {
        _printList(o.actions, ';', true);
        *(_stream) << ";";
    }

    return 0;
}

auto VerilogPrinter::visitString(String &o) -> int
{
    *(_stream) << "string";
    if (o.getSpanInformation() != nullptr) {
        *(_stream) << " ( ";
        o.getSpanInformation()->acceptVisitor(*this);
        *(_stream) << " )";
    }

    return 0;
}

auto VerilogPrinter::visitStateTable(StateTable &o) -> int
{
    //_printComment( &o );

    _stream->newLine();
    if (!_isSubProgramBody && !_ams_enabled) {
        // Print the process name if it is not equals to ""
        if (o.getName() != hif::NameTable::getInstance()->none()) {
            *(_stream) << o.getName() << ": ";
        }

        // StateTable Declaration and sensitivity list
        *(_stream) << "PROCESS";
        if (!o.sensitivity.empty()) {
            *(_stream) << "( ";
            _printList(o.sensitivity, ',', false);
            *(_stream) << " )";
        }
        *(_stream) << '\n';
        _stream->newLine();
    }

    // Print StateTable signal/variable declarations
    if (!o.declarations.empty()) {
        _stream->indent();
        _printList(o.declarations, ';', true);
        *_stream << ";";
        _stream->newLine();
        _stream->unindent();
        _stream->newLine();
    }

    _stream->indent();
    // StateTable body
    if (_ams_enabled && (processFlavourToString(o.getFlavour()) == "ANALOG")) {
        *(_stream) << "analog begin";
    } else {
        *(_stream) << "BEGIN" << '\n';
    }
    _stream->indent();
    _stream->newLine();

    messageAssert(o.edges.empty(), "Not empty edges list", &o, _sem);
    messageAssert(o.states.size() == 1, "Unsupported multiple states", &o, _sem);
    o.states.front()->acceptVisitor(*this);

    _stream->unindent();
    if (_ams_enabled) {
        *(_stream) << "\nend\n\n";
    } else if (!_isSubProgramBody) {
        *(_stream) << "\nEND PROCESS\n";
    } else {
        *(_stream) << "\nEND\n";
    }

    _stream->unindent();

    return 0;
}

auto VerilogPrinter::visitSystem(System &o) -> int
{
    //_printComment( &o );

    _currentSystem = &o;

    visitList(o.libraryDefs);
    visitList(o.designUnits);

    messageAssert(o.libraries.empty(), "Unsupported global libraries", nullptr, _sem);
    messageAssert(o.declarations.empty(), "Unsupported global declarations", nullptr, _sem);

    return 0;
}

auto VerilogPrinter::visitSwitchAlt(SwitchAlt &o) -> int
{
    GuideVisitor::visitSwitchAlt(o);
    return 0;
}

auto VerilogPrinter::visitSwitch(Switch &o) -> int
{
    GuideVisitor::visitSwitch(o);
    return 0;
}

auto VerilogPrinter::visitStringValue(StringValue &o) -> int
{
    GuideVisitor::visitStringValue(o);
    return 0;
}

auto VerilogPrinter::visitTime(Time &o) -> int { messageError("Time is not implemented yet.", &o, nullptr); }

auto VerilogPrinter::visitTimeValue(TimeValue &o) -> int
{
    messageError("TimeValue is not implemented yet.", &o, nullptr);
    return 0;
}

auto VerilogPrinter::visitTypeDef(TypeDef &o) -> int
{
    GuideVisitor::visitTypeDef(o);
    return 0;
}

auto VerilogPrinter::visitTypeReference(TypeReference &o) -> int
{
    GuideVisitor::visitTypeReference(o);
    return 0;
}

auto VerilogPrinter::visitTypeTPAssign(TypeTPAssign &o) -> int
{
    messageError("TypeTPAssign is not implemented yet.", &o, nullptr);
}

auto VerilogPrinter::visitTypeTP(TypeTP &o) -> int { messageError("TypeTP is not implemented yet.", &o, nullptr); }

auto VerilogPrinter::visitUnsigned(Unsigned &o) -> int
{
    *(_stream) << "unsigned";

    if (dynamic_cast<Cast *>(o.getParent()) != nullptr) {
        return 0;
    }
    if (dynamic_cast<Function *>(o.getParent()) != nullptr) {
        return 0;
    }

    if (o.getSpan() != nullptr) {
        *(_stream) << "( ";
        o.getSpan()->acceptVisitor(*this);
        *(_stream) << " )";
    }

    return 0;
}

auto VerilogPrinter::visitValueStatement(ValueStatement &o) -> int
{
    GuideVisitor::visitValueStatement(o);
    return 0;
}

auto VerilogPrinter::visitValueTPAssign(ValueTPAssign &o) -> int
{
    *(_stream) << o.getName() << " = ";

    o.getValue()->acceptVisitor(*this);
    return 0;
}

auto VerilogPrinter::visitValueTP(ValueTP &o) -> int
{
    *(_stream) << "\n";
    _stream->indent();
    *(_stream) << "parameter ";

    o.getType()->acceptVisitor(*this);

    *(_stream) << o.getName();

    if (o.getRange() != nullptr) {
        *(_stream) << " ";

        o.getRange()->acceptVisitor(*this);
    }

    if (o.getValue() != nullptr) {
        *(_stream) << " = ";
        o.getValue()->acceptVisitor(*this);
        *(_stream) << ";";
    }
    _stream->unindent();

    return 0;
}

auto VerilogPrinter::visitVariable(Variable &o) -> int
{
    if (_printFileVariable(&o)) {
        return 0;
    }
    _stream->indent();

    if (!_ams_enabled) {
        *(_stream) << "assign "; //TODO check
    } else {
        auto *viewRef = dynamic_cast<ViewReference *>(o.getType());
        if (viewRef != nullptr && viewRef->getName() == "ams_discipline") {
            assert(viewRef);
            std::string prefix("hif_verilog_");
            std::string currentType = viewRef->getDesignUnit();

            if (currentType.compare(0, prefix.size(), prefix) == 0) {
                *(_stream) << currentType.substr(prefix.length()) + " " + o.getName() + ";";
            }
        } else if (dynamic_cast<Real *>(o.getType()) != nullptr) {
            *(_stream) << "real " + o.getName() + ";";
        } else {
            *(_stream) << o.getName() << " = ";
        }
    }

    //o.getType()->acceptVisitor(*this);

    if (o.getRange() != nullptr) {
        *(_stream) << " range ";

        o.getRange()->acceptVisitor(*this);
    }

    if (o.getValue() != nullptr && !_ams_enabled) {
        //*(_stream) << " := ";

        o.getValue()->acceptVisitor(*this);
    }

    *(_stream) << "\n";
    _stream->unindent();

    return 0;
}

auto VerilogPrinter::visitView(View &o) -> int
{
    ////return 0;
    //_printComment( &o );

    _currentViewName = o.getName();
    _currentView     = &o;

    //for (auto TP : o.templateParameters) {
    //    messageInfo("" + TP->getName());
    //}

    // Get the contents of the current view
    Contents *cnt = o.getContents();
    Entity *en    = o.getEntity();

    // Print libraries
    //_printLibraries(o.libraries);

    _stream->newLine();

    // Visit the interface
    messageAssert(en != nullptr, "Unexpected nullptr entity", &o, _sem);
    en->acceptVisitor(*this);

    //if (!_isPrintingLibDefDecls) *_stream << ";" << std::endl;

    _stream->newLine();

    // Visit the contents
    if (cnt != nullptr) {
        cnt->acceptVisitor(*this);
    }

    return 0;
}

auto VerilogPrinter::visitViewReference(ViewReference &o) -> int
{
    GuideVisitor::visitViewReference(o);
    return 0;
}

auto VerilogPrinter::visitWait(Wait &o) -> int
{
    GuideVisitor::visitWait(o);
    return 0;
}

auto VerilogPrinter::visitWhen(When &o) -> int
{
    GuideVisitor::visitWhen(o);
    return 0;
}

auto VerilogPrinter::visitWhenAlt(WhenAlt &o) -> int
{
    GuideVisitor::visitWhenAlt(o);
    return 0;
}

auto VerilogPrinter::visitWhile(While &o) -> int
{
    if (o.getName() != NameTable::getInstance()->none()) {
        *(_stream) << o.getName() << ": ";
    }

    *(_stream) << "while ";
    o.getCondition()->acceptVisitor(*this);
    //*(_stream) << " LOOP" << endl;

    _stream->indent();
    _printList(o.actions, ";", true);
    _stream->unindent();

    *(_stream) << "end";
    return 0;
}

auto VerilogPrinter::visitWith(With &o) -> int
{
    messageInfo("Statement not supported in Verilog/A-MS");
    GuideVisitor::visitWith(o);
    return 0;
}

auto VerilogPrinter::visitWithAlt(WithAlt &o) -> int
{
    messageInfo("Statement not supported in Verilog/A-MS");
    GuideVisitor::visitWithAlt(o);
    return 0;
}

auto VerilogPrinter::_isSupportDeclaration(Declaration *d) -> bool
{
    if (dynamic_cast<Variable *>(d) != nullptr) {
        auto *v           = dynamic_cast<Variable *>(d);
        std::string vName = v->getName();
        if (_startsWith(vName, "old_") || _endsWith(vName, "_sig_var")) {
            return true;
        }
    }
    if (dynamic_cast<Procedure *>(d) != nullptr) {
        auto *p           = dynamic_cast<Procedure *>(d);
        std::string pName = p->getName();
        if (_startsWith(pName, "hif_cone_")) {
            return true;
        }
    }

    return false;
}

auto VerilogPrinter::_startsWith(const std::string &str, const std::string &target) -> bool
{
    std::size_t pos = str.find(target);
    if (pos == std::string::npos) {
        return false;
    }
    return (pos == 0);
}

auto VerilogPrinter::_endsWith(const std::string &str, const std::string &target) -> bool
{
    std::size_t pos = str.find(target);
    if (pos == std::string::npos) {
        return false;
    }
    return (pos == (str.length() - target.length()));
}

template <typename T> void VerilogPrinter::_printList(BList<T> &list, const char separator, bool needNewLine)
{
    auto *o = reinterpret_cast<BList<Object> *>(&list);
    _printList(*o, std::string() + separator, needNewLine);
}

template <typename T> void VerilogPrinter::_printList(BList<T> &list, const std::string &separator, bool needNewLine)
{
    auto *o = reinterpret_cast<BList<Object> *>(&list);
    _printList(*o, separator, needNewLine);
}

void VerilogPrinter::_printList(BList<Object> &list, const std::string &separator, bool needNewLine)
{
    if (list.empty()) {
        return;
    }

    for (BList<Object>::iterator it(list.begin()); it != list.end(); ++it) {
        if (it != list.begin()) {
            if (separator != " ") {
                *(_stream) << separator << " ";
            } else if (!needNewLine) {
                *(_stream) << " ";
            }

            if (needNewLine && _stream != nullptr) {
                _stream->newLine();
            }
        }

        (*it)->acceptVisitor(*this);
    }
}

void VerilogPrinter::_printValueInstance(Value *v)
{
    if (v == nullptr) {
        return;
    }

    bool needParen = (dynamic_cast<Expression *>(v) != nullptr);
    if (needParen) {
        *(_stream) << "(";
    }
    v->acceptVisitor(*this);
    if (needParen) {
        *(_stream) << ")";
    }

    bool printDot = false;

    auto *inst = dynamic_cast<Instance *>(v);
    if (inst != nullptr) {
        auto *lib = dynamic_cast<Library *>(inst->getReferencedType());
        if (lib != nullptr && lib->isStandard()) {
            return;
        }

        printDot = true;
    }

    if (printDot) {
        *(_stream) << ".";
    } else {
        *(_stream) << "'";
    }
}

void VerilogPrinter::_printTypeInstance(ReferencedType *v)
{
    if (v == nullptr) {
        return;
    }

    auto *lib = dynamic_cast<Library *>(v);
    if (lib != nullptr && lib->isStandard()) {
        return;
    }

    v->acceptVisitor(*this);

    *(_stream) << ".";
}

void VerilogPrinter::_printPortDirection(PortDirection dir)
{
    switch (dir) {
    case dir_in:
        *(_stream) << "in";
        break;
    case dir_out:
        *(_stream) << "out";
        break;
    case dir_inout:
        *(_stream) << "inout";
        break;
    case dir_none:
    default:
        messageAssert(dir != dir_none, "Not valid port direction", nullptr, _sem);
    }
}

void VerilogPrinter::_printLibraries(BList<Library> &libraries)
{
    bool libDecl = false;
    std::string libraryName;

    // First print the eventual IEEE libraries
    for (BList<Library>::iterator it = libraries.begin(); it != libraries.end(); ++it) {
        Library *lib = *it;
        libraryName  = lib->getName();
        if (lib->isStandard()) {
            continue;
        }

        if (libraryName == "standard") {
            continue;
        }

        if (!libDecl) {
            auto *terminal = dynamic_cast<Library *>(hif::getTerminalInstance(lib));
            if (terminal->getName() == "ieee") {
                *(_stream) << "library IEEE;";
                _stream->newLine();
                libDecl = true;
            }
        }

        *(_stream) << "use ";
        lib->acceptVisitor(*this);
        *(_stream) << ".all;";
        _stream->newLine();
    }
}

auto VerilogPrinter::_printFileVariable(Variable *o) -> bool
{
    File *f = dynamic_cast<File *>(hif::semantics::getBaseType(o->getType(), false, _sem));
    if (f == nullptr) {
        return false;
    }

    FunctionCall *fc = nullptr;
    if (o->getValue() != nullptr) {
        fc = dynamic_cast<FunctionCall *>(o->getValue());
        if (fc == nullptr || fc->getName() != "file_open" || fc->parameterAssigns.size() != 2) {
            return false;
        }
    }

    *(_stream) << "file " << o->getName() << ": ";

    o->getType()->acceptVisitor(*this);

    if (o->getValue() != nullptr) {
        messageAssert(fc != nullptr, "Unexpected case", nullptr, nullptr);
        ParameterAssign *p1 = fc->parameterAssigns.front();
        ParameterAssign *p2 = fc->parameterAssigns.back();

        *(_stream) << " open ";
        p2->getValue()->acceptVisitor(*this);
        *(_stream) << " is ";
        p1->getValue()->acceptVisitor(*this);
    }

    return true;
}

auto VerilogPrinter::_printAssertStatement(ProcedureCall *o) -> bool
{
    // void ASSERT(bool CONDITION, std::string REPORT = std::string(), severity_level LEVEL = NOTE)

    if (o->getName() != "assert") {
        return false;
    }
    auto size = o->parameterAssigns.size();
    if (size < 1 || size > 3) {
        return false;
    }

    *(_stream) << "assert ";

    ParameterAssign *p1 = o->parameterAssigns.at(0);
    ParameterAssign *p2 = nullptr;
    if (size > 1) {
        p2 = o->parameterAssigns.at(1);
    }
    ParameterAssign *p3 = nullptr;
    if (size > 2) {
        p3 = o->parameterAssigns.at(2);
    }

    p1->getValue()->acceptVisitor(*this);

    if (p2 != nullptr) {
        *(_stream) << " " << p2->getName() << " ";
        p2->getValue()->acceptVisitor(*this);
    }

    if (p3 != nullptr) {
        *(_stream) << " " << p3->getName() << " ";
        p3->getValue()->acceptVisitor(*this);
    }

    return true;
}

// return true if the difference between a and b is within epsilon percent of the larger of a and b
auto VerilogPrinter::_approximatelyEqual(double a, double b, double epsilon) -> bool
{
    return (std::abs(a - b) <= (std::max(std::abs(a), std::abs(b)) * epsilon));
}

void VerilogPrinter::processDeclarations(Contents &o)
{
    if (!o.declarations.empty()) {
        for (const auto &declaration : o.declarations) {
            if (this->_isSupportDeclaration(declaration)) {
                continue;
            }
            if (!_ams_enabled) {
                this->processNonAMSDeclaration(declaration);
            } else {
                this->processAMSDeclaration(declaration);
            }
        }
    }
}

void VerilogPrinter::processNonAMSDeclaration(Declaration *decl)
{
    if (auto *v = dynamic_cast<Variable *>(decl)) {
        this->printWire(v);
    } else if (auto *s = dynamic_cast<Signal *>(decl)) {
        this->printReg(s);
    }
}

void VerilogPrinter::printWire(Variable *v)
{
    *(_stream) << "\twire ";
    this->printBitwidth(v->getType());
    *(_stream) << v->getName();
    this->printValue(v->getValue());
    *(_stream) << ";\n";
}

void VerilogPrinter::printReg(Signal *s)
{
    *(_stream) << "\treg ";
    this->printBitwidth(s->getType());
    *(_stream) << s->getName();
    this->printValue(s->getValue());
    *(_stream) << ";\n";
}

void VerilogPrinter::printBitwidth(Type *type)
{
    unsigned long long bw = hif::semantics::typeGetSpanBitwidth(type, _sem);
    if (bw != 1) {
        *(_stream) << "[" << (bw - 1) << ":0] ";
    }
}

void VerilogPrinter::printValue(Value *value)
{
    if (auto *bv = dynamic_cast<BitValue *>(value)) {
        if (bv->getValue() != BitConstant::bit_x) {
            *(_stream) << " = " << bv->toString();
        }
    } else if (auto *bvv = dynamic_cast<BitvectorValue *>(value)) {
        if (!bvv->isX()) {
            *(_stream) << " = " << bvv->getValue();
        }
    }
}

void VerilogPrinter::processAMSDeclaration(Declaration *decl)
{
    if (auto *a = dynamic_cast<Alias *>(decl)) {
        printAlias(a);
    } else if (auto *v = dynamic_cast<Variable *>(decl)) {
        printAMSVariable(v);
    }
}

void VerilogPrinter::printAlias(Alias *a)
{
    _stream->indent();
    *(_stream) << "branch";
    a->getValue()->acceptVisitor(*this);
    *(_stream) << " " << a->getName() << ";\n";
    _stream->unindent();
}

void VerilogPrinter::printAMSVariable(Variable *v)
{
    std::string pName = v->getName();
    if (!pName.empty()) {
        _stream->indent();
        if (auto *portType = dynamic_cast<ViewReference *>(v->getType())) {
            printAMSViewReference(portType, pName);
        } else if (auto *portTypeRef = dynamic_cast<TypeReference *>(v->getType())) {
            printAMSTypeReference(portTypeRef, pName);
        }
        _stream->unindent();
    }
}

void VerilogPrinter::printAMSViewReference(ViewReference *portType, const std::string &pName)
{
    std::string prefix("hif_verilog_");
    std::string currentType = portType->getDesignUnit();
    if (currentType.compare(0, prefix.size(), prefix) == 0) {
        *(_stream) << currentType.substr(prefix.length()) + " " + pName << ";\n";
    }
}

void VerilogPrinter::printAMSTypeReference(TypeReference *portTypeRef, const std::string &pName)
{
    std::string prefix("hif_verilog_");
    std::string currentType = portTypeRef->getName();
    if (currentType.compare(0, prefix.size(), prefix) == 0) {
        *(_stream) << currentType.substr(prefix.length()) + " " + pName << ";\n";
    }
    if (auto *portTypeRef2 = portTypeRef->templateParameterAssigns.findByName("T")) {
        auto *viewRef2 = dynamic_cast<hif::TypeTPAssign *>(portTypeRef2)->getType();
        auto typeRef2  = dynamic_cast<hif::ViewReference *>(viewRef2)->getDesignUnit();
        assert(!typeRef2.empty());
        currentType = typeRef2;
        if (currentType.compare(0, prefix.size(), prefix) == 0) {
            *(_stream) << currentType.substr(prefix.length()) + " " + pName << ";\n";
        }
    }
}

void VerilogPrinter::processStateTables(Contents &o)
{
    if (!o.stateTables.empty() && _ams_enabled) {
        for (const auto &state_table : o.stateTables) {
            _stream->indent();
            _stream->newLine();
            *(_stream) << "analog begin\n";
            auto *process = state_table->states.findByName("process");
            if (process != nullptr) {
                _stream->indent();
                process->acceptVisitor(*this);
                _stream->unindent();
            }
            *(_stream) << "\nend";
            _stream->unindent();
        }
    }
}
