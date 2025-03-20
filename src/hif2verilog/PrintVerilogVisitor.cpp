/// @file PrintVerilogVisitor.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include <algorithm>
#include <cmath>

#include "hif2verilog/PrintVerilogVisitor.hpp"
//#include "hif2verilog/PrintVerilogMethods.hpp"

PrintVerilogVisitor::PrintVerilogVisitor(
    hif::backends::IndentedStream *outstream,
    const std::string &baseName,
    const std::string &extension)
    : _sem(hif::semantics::VerilogSemantics::getInstance())
    , _outstream(outstream)
    , _baseName(baseName)
    , _extension(extension)
{
    // empty
}

PrintVerilogVisitor::~PrintVerilogVisitor()
{
    // empty
}

// Namespace standard library
using namespace std;

// Namespace hifsuite
using namespace hif;

// Check
int PrintVerilogVisitor::visitAggregate(Aggregate &o)
{
    GuideVisitor::visitAggregate(o);
    return 0;
}

// Check
int PrintVerilogVisitor::visitAggregateAlt(AggregateAlt &o)
{
    GuideVisitor::visitAggregateAlt(o);
    return 0;
}

// Check
int PrintVerilogVisitor::visitAlias(Alias &o)
{
    GuideVisitor::visitAlias(o);
    return 0;
}

// Check
int PrintVerilogVisitor::visitArray(Array &o)
{
    *(_outstream) << "array( ";
    // Print Span
    o.getSpan()->acceptVisitor(*this);
    *(_outstream) << ") ";
    // Print Type
    *(_outstream) << "of ";
    o.getType()->acceptVisitor(*this);

    return 0;
}

int PrintVerilogVisitor::visitAssign(Assign &o)
{
    // handle With assign
    if (dynamic_cast<With *>(o.getRightHandSide()) != nullptr) {
        _isPrintWithCondition = true;
        o.getRightHandSide()->acceptVisitor(*this);
        _isPrintWithCondition = false;

        _outstream->newLine();
        _outstream->indent();
    }

    _outstream->indent();

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
    //    *(_outstream) << " <+ ";
    //} // if the target is a variable put := otherwise put <=
    if (dynamic_cast<Variable *>(dd)) {
        *(_outstream) << " = ";
    } else {
        *(_outstream) << " <= ";
    }

    o.getRightHandSide()->acceptVisitor(*this);
    _outstream->unindent();

    return 0;
}

// Check std_logic and std_ulogic
int PrintVerilogVisitor::visitBit(Bit &o)
{
    if (o.isLogic()) {
        if (o.isResolved()) {
            *(_outstream) << "std_logic";
        } else {
            *(_outstream) << "std_ulogic";
        }
    } else {
        *(_outstream) << "bit";
    }

    return 0;
}

// Verilog has four logic values, 0,1 Z, X. Values U, W, L, H, - are only in VHDL.
int PrintVerilogVisitor::visitBitValue(BitValue &o)
{
    switch (o.getValue()) {
    case bit_zero:
        *(_outstream) << "'0'";
        break;
    case bit_one:
        *(_outstream) << "'1'";
        break;
    case bit_x:
        *(_outstream) << "'X'";
        break;
    case bit_z:
        *(_outstream) << "'Z'";
        break;
    case bit_u:
        *(_outstream) << "'U'";
        break;
    case bit_w:
        *(_outstream) << "'W'";
        break;
    case bit_l:
        *(_outstream) << "'L'";
        break;
    case bit_h:
        *(_outstream) << "'H'";
        break;
    case bit_dontcare:
        *(_outstream) << "'-'";
        break;
    default: // unsupported
        messageError("Unexpected bit value", &o, _sem);
    }

    return 0;
}

int PrintVerilogVisitor::visitBitvector(Bitvector &o)
{
    GuideVisitor::visitBitvector(o);
    return 0;
}

int PrintVerilogVisitor::visitBitvectorValue(BitvectorValue &o)
{
    GuideVisitor::visitBitvectorValue(o);
    return 0;
}

int PrintVerilogVisitor::visitBool(Bool &o)
{
    GuideVisitor::visitBool(o);
    return 0;
}

int PrintVerilogVisitor::visitBoolValue(BoolValue &o)
{
    GuideVisitor::visitBoolValue(o);
    return 0;
}

int PrintVerilogVisitor::visitBreak(Break &o)
{
    GuideVisitor::visitBreak(o);
    return 0;
}

int PrintVerilogVisitor::visitCast(Cast &o)
{
    GuideVisitor::visitCast(o);
    return 0;
}

int PrintVerilogVisitor::visitChar(Char &o)
{
    GuideVisitor::visitChar(o);
    return 0;
}

int PrintVerilogVisitor::visitCharValue(CharValue &o)
{
    GuideVisitor::visitCharValue(o);
    return 0;
}

int PrintVerilogVisitor::visitConst(Const &o)
{
    /*
    *(_outstream) << "\tlocalparam " << o.getName();

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
                *(_outstream) << " = " << iv->getValue();
            } else {
             *(_outstream) << " = " << bvv->getValue();
            }
        } else
            messageError("Unhandled localparam value", &o, _sem);
    }
    *(_outstream) << ";\n";
    */
    GuideVisitor::visitConst(o);
    return 0;
}

int PrintVerilogVisitor::visitContents(Contents &o)
{
    if (o.getName() == "Architecture") {
        this->processDeclarations(o);
        this->processStateTables(o);
    }
    return 0;
}

int PrintVerilogVisitor::visitContinue(Continue &o)
{
    GuideVisitor::visitContinue(o);
    return 0;
}

int PrintVerilogVisitor::visitDesignUnit(DesignUnit &o)
{
#if 0
    if (hif::languageIDToString(o.views.at(0)->getLanguageID()) == "AMS") {
        _ams_enabled = true;
        messageInfo("AMS enabled into the PrintVerilogVisitor");
    }
#endif

    auto duName            = o.getName();
    _currentDesignUnitName = duName;
    *(_outstream) << "module " << duName;
    messageAssert(o.views.size() == 1, "Not supported more than one view", &o, _sem);
    GuideVisitor::visitDesignUnit(o);
    _outstream->newLine(1);
    *(_outstream) << "endmodule // " << duName << "\n";
    return 0;
}

int PrintVerilogVisitor::visitEnum(Enum &o)
{
    *(_outstream) << "(";
    _printList(o.values, ',', false);
    *(_outstream) << ")";

    return 0;
}

int PrintVerilogVisitor::visitEnumValue(EnumValue &o)
{
    *(_outstream) << o.getName();
    return 0;
}

int PrintVerilogVisitor::visitEvent(Event &o)
{
    GuideVisitor::visitEvent(o);
    return 0;
}

int PrintVerilogVisitor::visitExpression(Expression &o)
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
        if (needOp1Paren)
            *(_outstream) << "(";
        o.getValue1()->acceptVisitor(*this);
        if (needOp1Paren)
            *(_outstream) << ")";
        *(_outstream) << " ";
    }

    // Print operator.
    switch (o.getOperator()) {
    // Logical (boolean) operators
    case op_not:
        *(_outstream) << "!";
        break;
    case op_or:
        *(_outstream) << "||";
        break;
    case op_xor:
        *(_outstream) << "^";
        break;
    case op_and:
        *(_outstream) << "&&";
        break;

        // Binary (bitwise) operators
    case op_bnot:
        *(_outstream) << "~";
        break;
    case op_bor:
        *(_outstream) << "|";
        break;
    case op_bxor:
        *(_outstream) << "^";
        break;
    case op_band:
        *(_outstream) << "&";
        break;
    case op_sll:
        *(_outstream) << "<<";
        break;
    case op_srl:
        *(_outstream) << ">>";
        break;
    case op_rol:
        *(_outstream) << "rol"; // TODO
        break;
    case op_ror:
        *(_outstream) << "ror"; // TODO
        break;

        // Concatenation operator
    case op_concat:
        *(_outstream) << "{ }";
        break;

        // Equality operators
    case op_eq:
    case op_case_eq:
        *(_outstream) << "=";
        break;
    case op_neq:
    case op_case_neq:
        *(_outstream) << "!=";
        break;

        // Relational operators
    case op_lt:
        *(_outstream) << "<";
        break;
    case op_le:
        *(_outstream) << "<=";
        break;
    case op_gt:
        *(_outstream) << ">";
        break;
    case op_ge:
        *(_outstream) << ">=";
        break;

        // Arithmetic operators
    case op_plus:
        *(_outstream) << "+";
        break;
    case op_minus:
        *(_outstream) << "-";
        break;
    case op_mult:
        *(_outstream) << "*";
        break;
    case op_div:
        *(_outstream) << "/";
        break;
    case op_rem:
        *(_outstream) << "rem"; // TODO
        break;
    case op_pow:
        *(_outstream) << "**";
        break;
    case op_abs:
        *(_outstream) << "abs"; // TODO
        break;
    case op_ref:
        break; // TODO
    case op_deref:
        break; // TODO
    case op_sla:
        *(_outstream) << "sla"; // TODO
        break;
    case op_sra:
        *(_outstream) << "sra"; // TODO
        break;
    case op_mod:
        *(_outstream) << "%";
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
    *(_outstream) << " ";

    // If binary expression, print Op2.
    if (o.getValue2() != nullptr) {
        if (needOp2Paren)
            *(_outstream) << "(";
        o.getValue2()->acceptVisitor(*this);
        if (needOp2Paren)
            *(_outstream) << ")";
    }
    // If unary expression, print Op1.
    else {
        if (needOp1Paren)
            *(_outstream) << "(";
        o.getValue1()->acceptVisitor(*this);
        if (needOp1Paren)
            *(_outstream) << ")";
    }
    return 0;
}

int PrintVerilogVisitor::visitFunctionCall(FunctionCall &o)
{
    // handle attributes
    //_printValueInstance(o.getInstance());

    if (dynamic_cast<Alias *>(o.getParent()) == nullptr) {
        if (o.getName() == "hif_verilog_V") {
            *(_outstream) << "V";
        } else if (o.getName() == "hif_verilog_I") {
            *(_outstream) << "I";
        } else if (o.getName() == "hif_verilog_ddt") {
            *(_outstream) << "ddt";
        } else if (o.getName() == "hif_verilog_Omega") {
            *(_outstream) << "Omega";
        } else if (o.getName() == "hif_verilog_Tau") {
            *(_outstream) << "Tau";
        } else {
            *(_outstream) << o.getName();
        }

        // Print parameters
        if (!o.parameterAssigns.empty()) {
            *(_outstream) << "(";
            _printList(o.parameterAssigns, ',', false);
            *(_outstream) << ")";
        }
    } else {
        // Print parameters
        if (!o.parameterAssigns.empty()) {
            *(_outstream) << "(";
            _printList(o.parameterAssigns, ',', false);
            *(_outstream) << ")";
        }
    }

    return 0;
}

int PrintVerilogVisitor::visitField(Field &o)
{
    GuideVisitor::visitField(o);
    return 0;
}

int PrintVerilogVisitor::visitFieldReference(FieldReference &o)
{
    GuideVisitor::visitFieldReference(o);
    return 0;
}

int PrintVerilogVisitor::visitFile(File &o)
{
    GuideVisitor::visitFile(o);
    return 0;
}

int PrintVerilogVisitor::visitFor(For &o)
{
    GuideVisitor::visitFor(o);
    return 0;
}

int PrintVerilogVisitor::visitForGenerate(ForGenerate &o)
{
    GuideVisitor::visitForGenerate(o);
    return 0;
}

int PrintVerilogVisitor::visitFunction(Function &o)
{
    GuideVisitor::visitFunction(o);
    return 0;
}

int PrintVerilogVisitor::visitGlobalAction(GlobalAction &o)
{
    GuideVisitor::visitGlobalAction(o);
    return 0;
}

int PrintVerilogVisitor::visitEntity(Entity &o)
{
    _currentEntity = &o;

    if (!o.ports.empty()) {
        // Print Ports of the module
        *(_outstream) << "(" << endl;
        _outstream->indent();
        for (auto it : o.ports) {
            auto pName = (it->getName());
            if (pName == o.ports.at(0)->getName())
                *(_outstream) << pName;
            else
                *(_outstream) << ", " << pName;
        }
        *(_outstream) << ";" << endl;
        _outstream->newLine();
        _outstream->unindent();
        *(_outstream) << ");" << endl;

        //TODO
        // Print port declarations
        _outstream->newLine();
        _outstream->indent();
        for (auto it2 : o.ports) {
            auto pName = it2->getName();
            assert(!pName.empty());
            auto portType = dynamic_cast<ViewReference *>(it2->getType());
            if (portType != nullptr) {
                std::string prefix("hif_verilog_");
                std::string currentType = portType->getDesignUnit();

                if (!currentType.compare(0, prefix.size(), prefix)) {
                    *(_outstream) << currentType.substr(prefix.length()) + " " + pName << ";\n";
                }
            } else {
                *(_outstream) << "<discipline_not_supported> " << pName << ";\n";
            }
            //if (portType) {
            //    if (portType->getDesignUnit() == "hif_verilog_voltage")
            //        *(_outstream) << "voltage " << pName << ";\n";
            //}
        }
        _outstream->unindent();
    }

    // Print Template Parameters
    for (auto tp : _currentView->templateParameters) {
        tp->acceptVisitor(*this);
    }

    *(_outstream) << "\n";

    // Port Declarations
    _outstream->indent();
    for (auto it3 : o.ports) {
        auto direction = it3->getDirection();
        if (direction == PortDirection::dir_in)
            *(_outstream) << "input ";
        else if (direction == PortDirection::dir_out)
            *(_outstream) << "output ";
        else if (direction == PortDirection::dir_inout)
            *(_outstream) << "inout ";
        else
            messageError("Unexpected PortDirection", it3, _sem);
        // TODO Add net_type (if any)
        // Add signed (if any)
        if (typeIsSigned(it3->getType(), _sem))
            *(_outstream) << "signed ";
        // Add range (if any))
        if (!_ams_enabled) {
            unsigned long long bw = hif::semantics::typeGetSpanBitwidth(it3->getType(), _sem);
            if (bw != 1)
                *(_outstream) << "[" << (bw - 1) << ":0] ";
        }
        *(_outstream) << it3->getName() << ";\n";
    }
    _outstream->unindent();

    return 0;
}

int PrintVerilogVisitor::visitIdentifier(Identifier &o)
{
    *(_outstream) << o.getName();

    return 0;
}

int PrintVerilogVisitor::visitIf(If &o)
{
    GuideVisitor::visitIf(o);
    return 0;
}

int PrintVerilogVisitor::visitIfAlt(IfAlt &o)
{
    GuideVisitor::visitIfAlt(o);
    return 0;
}

// TODO
int PrintVerilogVisitor::visitIfGenerate(IfGenerate &o)
{
    messageInfo("IfGenerate is not implemented yet.");

    GuideVisitor::visitIfGenerate(o);
    return 0;
}

int PrintVerilogVisitor::visitInstance(Instance &o)
{
    auto vr = dynamic_cast<ViewReference *>(o.getReferencedType());

    if (vr != nullptr) {
        _outstream->indent();
        if (vr->getName() == "behav") {
            if (!o.portAssigns.empty()) {
                *(_outstream) << vr->getDesignUnit() + " " + o.getName() + " (";
                // print ports
                _printList(o.portAssigns, ',', false);
                *(_outstream) << ");\n";
            }
        }
        _outstream->unindent();
    }

    return 0;
}

// TODO
int PrintVerilogVisitor::visitInt(Int &o)
{
    *(_outstream) << (o.isSigned() ? "integer" : "natural");

    return 0;
}

int PrintVerilogVisitor::visitIntValue(IntValue &o)
{
    *(_outstream) << o.getValue();
    if (_isRealRange) {
        *(_outstream) << ".0";
    }

    return 0;
}

int PrintVerilogVisitor::visitLibraryDef(LibraryDef &o)
{
    if (o.isStandard())
        return 0;

    string libraryDefName = o.getName();
    // Create the subdirectory
    string dirName        = _outDir + "/src/" + libraryDefName;
    _createDirectory(dirName);

    // Initialize the output stream
    _initializeOutstream(libraryDefName, libraryDefName + "/");

    *(_outstream) << "PACKAGE " << libraryDefName << " IS" << endl << endl;
    _outstream->indent();

    // Print LibraryDef content
    if (!o.declarations.empty()) {
        const bool restore     = _isPrintingLibDefDecls;
        _isPrintingLibDefDecls = true;
        _printList(o.declarations, ';', true);
        *(_outstream) << ";";
        _isPrintingLibDefDecls = restore;
    }

    _outstream->unindent();
    *(_outstream) << "\n\nEND " << libraryDefName << ";" << endl;

    *(_outstream) << "\n\nPACKAGE BODY " << libraryDefName << " IS" << endl << endl;
    _outstream->indent();

    // Custom printing to skip type defs.
    for (BList<Declaration>::iterator i = o.declarations.begin(); i != o.declarations.end(); ++i) {
        Declaration *d = *i;
        if (dynamic_cast<TypeDef *>(d) != nullptr)
            continue;
        d->acceptVisitor(*this);
        *(_outstream) << ";";
        _outstream->newLine();
    }

    _outstream->unindent();
    *(_outstream) << "\nEND " << libraryDefName << ";" << std::endl << std::flush;
    delete _outstream;
    _outstream = nullptr;
    return 0;
}

int PrintVerilogVisitor::visitLibrary(Library &o)
{
    if (o.isStandard())
        return 0;

    _printTypeInstance(o.getInstance());

    if (o.getInstance() == nullptr && !o.isStandard() && !o.isSystem()) {
        *(_outstream) << "work.";
    }

    // TODO
    //*(_outstream) << o.getName();

    return 0;
}

int PrintVerilogVisitor::visitMember(Member &o)
{
    const bool needParen = (dynamic_cast<Expression *>(o.getPrefix()) != nullptr);
    if (needParen)
        *(_outstream) << "(";
    o.getPrefix()->acceptVisitor(*this);
    if (needParen)
        *(_outstream) << ")";

    messageAssert(o.getIndex() != nullptr, "Unsupported member without index", &o, _sem);

    *(_outstream) << "( ";
    o.getIndex()->acceptVisitor(*this);
    *(_outstream) << " )";

    return 0;
}

int PrintVerilogVisitor::visitNull(Null & /*o*/)
{
    *(_outstream) << "nullptr";
    return 0;
}

int PrintVerilogVisitor::visitTransition(Transition &o)
{
    messageError("Transition is not implemented yet.", &o, nullptr);
}

int PrintVerilogVisitor::visitParameterAssign(ParameterAssign &o)
{
    if (o.getValue() != nullptr)
        o.getValue()->acceptVisitor(*this);

    return 0;
}

int PrintVerilogVisitor::visitParameter(Parameter &o)
{
    *(_outstream) << o.getName();
    *(_outstream) << ": ";

    if (o.getDirection() != dir_none) {
        _printPortDirection(o.getDirection());
        *(_outstream) << " ";
    }

    o.getType()->acceptVisitor(*this);

    if (o.getValue() != nullptr) {
        *(_outstream) << " := ";
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

int PrintVerilogVisitor::visitProcedureCall(ProcedureCall &o)
{
    Declaration *decl = hif::semantics::getDeclaration(&o, _sem);
    if (_isSupportDeclaration(decl))
        return 0;

    std::string name = o.getName();
    if (name == "hif_verilog_vams_indirect_contribution_statement") {
        assert(o.parameterAssigns.size() == 2);
        o.parameterAssigns.at(0)->acceptVisitor(*this);

        *(_outstream) << ": ";

        auto cast = dynamic_cast<Cast *>(o.parameterAssigns.at(1)->getValue());
        assert(cast);
        auto expression = dynamic_cast<Expression *>(cast->getValue());
        assert(expression);
        expression->getValue2()->acceptVisitor(*this);
        *(_outstream) << " == ";
        expression->getValue1()->acceptVisitor(*this);
    } else if (name == "hif_verilog_vams_contribution_statement") {
        //_outstream->indent();
        assert(o.parameterAssigns.size() == 2);
        o.parameterAssigns.at(0)->acceptVisitor(*this);

        *(_outstream) << " <+ ";

        auto expression = dynamic_cast<Expression *>(o.parameterAssigns.at(1)->getValue());
        if (expression != nullptr)
            expression->acceptVisitor(*this);
        //expression->getValue2()->acceptVisitor(*this);
        //*(_outstream) << " == ";
        //expression->getValue1()->acceptVisitor(*this);

        //_outstream->unindent();
    } else {
        // handle attributes
        //_printValueInstance(o.getInstance());

        *(_outstream) << o.getName();

        // Print parameters
        if (!o.parameterAssigns.empty()) {
            *(_outstream) << "(";
            _printList(o.parameterAssigns, ',', false);
            *(_outstream) << ")";
        }
    }
    return 0;
}

int PrintVerilogVisitor::visitPointer(Pointer &o)
{
    *(_outstream) << "access ";
    o.getType()->acceptVisitor(*this);

    return 0;
}

int PrintVerilogVisitor::visitPortAssign(PortAssign &o)
{
    //*(_outstream) << o.getName() << " => ";
    o.getValue()->acceptVisitor(*this);

    return 0;
}

int PrintVerilogVisitor::visitPort(Port &o)
{
    //_printComment( &o );

    // Name: direction type;

    *(_outstream) << o.getName() << ": ";

    _printPortDirection(o.getDirection());

    *(_outstream) << " ";

    o.getType()->acceptVisitor(*this);

    if (o.getRange() != nullptr) {
        *(_outstream) << " range ";
        o.getRange()->acceptVisitor(*this);
    }

    return 0;
}

int PrintVerilogVisitor::visitProcedure(Procedure &o)
{
    if (_isSupportDeclaration(&o))
        return 0;

    *(_outstream) << "PROCEDURE ";
    *(_outstream) << o.getName();

    if (!o.parameters.empty()) {
        *(_outstream) << " (";
        _printList(o.parameters, ';', false);
        *(_outstream) << " )";
    }

    if (!_isPrintingLibDefDecls) {
        *(_outstream) << " IS" << std::endl;
        const bool restore = _isSubProgramBody;
        _isSubProgramBody  = true;
        o.getStateTable()->acceptVisitor(*this);
        _isSubProgramBody = restore;
    }

    return 0;
}

int PrintVerilogVisitor::visitRange(Range &o)
{
    const bool restore = _isRealRange;
    _setRealRange(&o);

    o.getLeftBound()->acceptVisitor(*this);
    switch (o.getDirection()) {
    case dir_downto:
        *(_outstream) << " downto ";
        break;
    case dir_upto:
        *(_outstream) << " to ";
        break;
    default:
        messageError("Unsupported range", &o, _sem);
    }
    o.getRightBound()->acceptVisitor(*this);

    _isRealRange = restore;
    return 0;
}

int PrintVerilogVisitor::visitReal(Real & /*o*/)
{
    *(_outstream) << "real ";

    return 0;
}

int PrintVerilogVisitor::visitRealValue(RealValue &o)
{
    *(_outstream) << o.getValue();

    double whole, decimal;
    decimal = std::modf(o.getValue(), &whole);
    if (_approximatelyEqual(decimal, 0.0, 0.001)) {
        //if (decimal == 0.0) {
        *(_outstream) << ".0";
    }

    return 0;
}

int PrintVerilogVisitor::visitRecord(Record &o)
{
    *(_outstream) << "RECORD" << endl;
    _outstream->indent();
    _printList(o.fields, ';', true);
    _outstream->unindent();
    *(_outstream) << "END RECORD" << endl;
    return 0;
}

int PrintVerilogVisitor::visitRecordValue(RecordValue &o)
{
    *(_outstream) << "( ";

    _printList(o.alts, ',', true);

    *(_outstream) << " )";

    return 0;
}

int PrintVerilogVisitor::visitRecordValueAlt(RecordValueAlt &o)
{
    *(_outstream) << o.getName() << " <= ";
    o.getValue()->acceptVisitor(*this);

    return 0;
}

int PrintVerilogVisitor::visitReference(Reference &o)
{
    messageError("Reference is not implemented yet.", &o, nullptr);
}

int PrintVerilogVisitor::visitReturn(Return &o)
{
    *(_outstream) << "return";
    if (o.getValue() != nullptr) {
        *(_outstream) << " ";
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

int PrintVerilogVisitor::visitSignal(Signal &o)
{
    // Check if AMS is enabled
    if (_ams_enabled) {
        _outstream->indent();
        auto currentView = dynamic_cast<ViewReference *>(o.getType());
        assert(currentView);
        // Check flavour because it could be a digital signal in an AMS design
        if (currentView->getName() == "ams_discipline") {
            std::string prefix("hif_verilog_");
            std::string currentType = currentView->getDesignUnit();

            if (!currentType.compare(0, prefix.size(), prefix)) {
                *(_outstream) << currentType.substr(prefix.length()) + " " + o.getName() + ";";
            }
        }
    } else {
        // signal identifier : subtype_indication [ := expression ];
        // E.g.: signal name: integer range 7 downto 0;
        _outstream->indent();
        *(_outstream) << "signal " << o.getName() << "; ";

        //o.getType()->acceptVisitor( *this );

        if (o.getRange() != nullptr) {
            *(_outstream) << " range ";
            o.getRange()->acceptVisitor(*this);
        }

        if (o.getValue() != nullptr) {
            *(_outstream) << " := ";

            o.getValue()->acceptVisitor(*this);
        }
    }

    *(_outstream) << "\n";
    _outstream->unindent();

    return 0;
}

int PrintVerilogVisitor::visitSigned(Signed &o)
{
    *(_outstream) << "signed";

    if (dynamic_cast<Cast *>(o.getParent()) != nullptr)
        return 0;
    if (dynamic_cast<Function *>(o.getParent()) != nullptr)
        return 0;

    if (o.getSpan()) {
        *(_outstream) << "( ";
        o.getSpan()->acceptVisitor(*this);
        *(_outstream) << " )";
    }

    return 0;
}

int PrintVerilogVisitor::visitSlice(Slice &o)
{
    const bool needParen = (dynamic_cast<Expression *>(o.getPrefix()) != nullptr);
    if (needParen)
        *(_outstream) << "(";
    o.getPrefix()->acceptVisitor(*this);
    if (needParen)
        *(_outstream) << ")";

    *(_outstream) << "( ";
    o.getSpan()->acceptVisitor(*this);
    *(_outstream) << " )";
    return 0;
}

int PrintVerilogVisitor::visitState(State &o)
{
    if (!o.actions.empty()) {
        _printList(o.actions, ';', true);
        *(_outstream) << ";";
    }

    return 0;
}

int PrintVerilogVisitor::visitString(String &o)
{
    *(_outstream) << "string";
    if (o.getSpanInformation() != nullptr) {
        *(_outstream) << " ( ";
        o.getSpanInformation()->acceptVisitor(*this);
        *(_outstream) << " )";
    }

    return 0;
}

int PrintVerilogVisitor::visitStateTable(StateTable &o)
{
    //_printComment( &o );

    _outstream->newLine();
    if (!_isSubProgramBody && !_ams_enabled) {
        // Print the process name if it is not equals to ""
        if (o.getName() != hif::NameTable::getInstance()->none()) {
            *(_outstream) << o.getName() << ": ";
        }

        // StateTable Declaration and sensitivity list
        *(_outstream) << "PROCESS";
        if (!o.sensitivity.empty()) {
            *(_outstream) << "( ";
            _printList(o.sensitivity, ',', false);
            *(_outstream) << " )";
        }
        *(_outstream) << std::endl;
        _outstream->newLine();
    }

    // Print StateTable signal/variable declarations
    if (!o.declarations.empty()) {
        _outstream->indent();
        _printList(o.declarations, ';', true);
        *_outstream << ";";
        _outstream->newLine();
        _outstream->unindent();
        _outstream->newLine();
    }

    _outstream->indent();
    // StateTable body
    if (_ams_enabled && (processFlavourToString(o.getFlavour()) == "ANALOG"))
        *(_outstream) << "analog begin";
    else
        *(_outstream) << "BEGIN" << endl;
    _outstream->indent();
    _outstream->newLine();

    messageAssert(o.edges.empty(), "Not empty edges list", &o, _sem);
    messageAssert(o.states.size() == 1, "Unsupported multiple states", &o, _sem);
    o.states.front()->acceptVisitor(*this);

    _outstream->unindent();
    if (_ams_enabled)
        *(_outstream) << "\nend\n\n";
    else if (!_isSubProgramBody)
        *(_outstream) << "\nEND PROCESS\n";
    else
        *(_outstream) << "\nEND\n";

    _outstream->unindent();

    return 0;
}

int PrintVerilogVisitor::visitSystem(System &o)
{
    //_printComment( &o );

    _currentSystem = &o;

    // Create the source directory
    string dirName = _outDir + "/src";
    _createDirectory(dirName);

    visitList(o.libraryDefs);
    visitList(o.designUnits);

    messageAssert(o.libraries.empty(), "Unsupported global libraries", nullptr, _sem);
    messageAssert(o.declarations.empty(), "Unsupported global declarations", nullptr, _sem);

    return 0;
}

int PrintVerilogVisitor::visitSwitchAlt(SwitchAlt &o)
{
    GuideVisitor::visitSwitchAlt(o);
    return 0;
}

int PrintVerilogVisitor::visitSwitch(Switch &o)
{
    GuideVisitor::visitSwitch(o);
    return 0;
}

int PrintVerilogVisitor::visitStringValue(StringValue &o)
{
    GuideVisitor::visitStringValue(o);
    return 0;
}

int PrintVerilogVisitor::visitTime(Time &o) { messageError("Time is not implemented yet.", &o, nullptr); }

int PrintVerilogVisitor::visitTimeValue(TimeValue &o)
{
    messageError("TimeValue is not implemented yet.", &o, nullptr);
    return 0;
}

int PrintVerilogVisitor::visitTypeDef(TypeDef &o)
{
    GuideVisitor::visitTypeDef(o);
    return 0;
}

int PrintVerilogVisitor::visitTypeReference(TypeReference &o)
{
    GuideVisitor::visitTypeReference(o);
    return 0;
}

int PrintVerilogVisitor::visitTypeTPAssign(TypeTPAssign &o)
{
    messageError("TypeTPAssign is not implemented yet.", &o, nullptr);
}

int PrintVerilogVisitor::visitTypeTP(TypeTP &o) { messageError("TypeTP is not implemented yet.", &o, nullptr); }

int PrintVerilogVisitor::visitUnsigned(Unsigned &o)
{
    *(_outstream) << "unsigned";

    if (dynamic_cast<Cast *>(o.getParent()) != nullptr)
        return 0;
    if (dynamic_cast<Function *>(o.getParent()) != nullptr)
        return 0;

    if (o.getSpan()) {
        *(_outstream) << "( ";
        o.getSpan()->acceptVisitor(*this);
        *(_outstream) << " )";
    }

    return 0;
}

int PrintVerilogVisitor::visitValueStatement(ValueStatement &o)
{
    GuideVisitor::visitValueStatement(o);
    return 0;
}

int PrintVerilogVisitor::visitValueTPAssign(ValueTPAssign &o)
{
    *(_outstream) << o.getName() << " = ";

    o.getValue()->acceptVisitor(*this);
    return 0;
}

int PrintVerilogVisitor::visitValueTP(ValueTP &o)
{
    *(_outstream) << "\n";
    _outstream->indent();
    *(_outstream) << "parameter ";

    o.getType()->acceptVisitor(*this);

    *(_outstream) << o.getName();

    if (o.getRange() != nullptr) {
        *(_outstream) << " ";

        o.getRange()->acceptVisitor(*this);
    }

    if (o.getValue() != nullptr) {
        *(_outstream) << " = ";
        o.getValue()->acceptVisitor(*this);
        *(_outstream) << ";";
    }
    _outstream->unindent();

    return 0;
}

int PrintVerilogVisitor::visitVariable(Variable &o)
{
    if (_printFileVariable(&o))
        return 0;
    _outstream->indent();

    if (!_ams_enabled) {
        *(_outstream) << "assign "; //TODO check
    } else {
        auto viewRef = dynamic_cast<ViewReference *>(o.getType());
        if (viewRef != nullptr && viewRef->getName() == "ams_discipline") {
            assert(viewRef);
            std::string prefix("hif_verilog_");
            std::string currentType = viewRef->getDesignUnit();

            if (!currentType.compare(0, prefix.size(), prefix)) {
                *(_outstream) << currentType.substr(prefix.length()) + " " + o.getName() + ";";
            }
        } else if (dynamic_cast<Real *>(o.getType()) != nullptr) {
            *(_outstream) << "real " + o.getName() + ";";
        } else {
            *(_outstream) << o.getName() << " = ";
        }
    }

    //o.getType()->acceptVisitor(*this);

    if (o.getRange() != nullptr) {
        *(_outstream) << " range ";

        o.getRange()->acceptVisitor(*this);
    }

    if (o.getValue() != nullptr && !_ams_enabled) {
        //*(_outstream) << " := ";

        o.getValue()->acceptVisitor(*this);
    }

    *(_outstream) << "\n";
    _outstream->unindent();

    return 0;
}

int PrintVerilogVisitor::visitView(View &o)
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

    _outstream->newLine();

    // Visit the interface
    messageAssert(en != nullptr, "Unexpected nullptr entity", &o, _sem);
    en->acceptVisitor(*this);

    //if (!_isPrintingLibDefDecls) *_outstream << ";" << std::endl;

    _outstream->newLine();

    // Visit the contents
    if (cnt != nullptr)
        cnt->acceptVisitor(*this);

    return 0;
}

int PrintVerilogVisitor::visitViewReference(ViewReference &o)
{
    GuideVisitor::visitViewReference(o);
    return 0;
}

int PrintVerilogVisitor::visitWait(Wait &o)
{
    GuideVisitor::visitWait(o);
    return 0;
}

int PrintVerilogVisitor::visitWhen(When &o)
{
    GuideVisitor::visitWhen(o);
    return 0;
}

int PrintVerilogVisitor::visitWhenAlt(WhenAlt &o)
{
    GuideVisitor::visitWhenAlt(o);
    return 0;
}

int PrintVerilogVisitor::visitWhile(While &o)
{
    if (o.getName() != NameTable::getInstance()->none()) {
        *(_outstream) << o.getName() << ": ";
    }

    *(_outstream) << "while ";
    o.getCondition()->acceptVisitor(*this);
    //*(_outstream) << " LOOP" << endl;

    _outstream->indent();
    _printList(o.actions, ";", true);
    _outstream->unindent();

    *(_outstream) << "end";
    return 0;
}

int PrintVerilogVisitor::visitWith(With &o)
{
    messageInfo("Statement not supported in Verilog/A-MS");
    GuideVisitor::visitWith(o);
    return 0;
}

int PrintVerilogVisitor::visitWithAlt(WithAlt &o)
{
    messageInfo("Statement not supported in Verilog/A-MS");
    GuideVisitor::visitWithAlt(o);
    return 0;
}

void PrintVerilogVisitor::_initializeOutstream(string fileName, string subdirectory)
{
    if (fileName.empty()) {
        messageError("Empty file name", nullptr, nullptr);
    }

    string path = _outDir + "/src/" + subdirectory + fileName;

    if (_outstream != nullptr)
        delete _outstream;

    _outstream = new hif::backends::IndentedStream(path, "vhd");
    _outstream->setComment("--", "--", "");

    //_printInitBanner();
}

int PrintVerilogVisitor::_createDirectory(string dirName)
{
    hif::application_utils::FileStructure dir(dirName);
    // Empty directory if it already exists.
    if (dir.exists()) {
        vector<string> fileList = dir.list();
        for (vector<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
            hif::application_utils::FileStructure fileIn(*it);
            fileIn.rmfile_weak();
        }
    }
    // Create new directory (if it does not already exist)
    else if (!dir.make_dir()) {
        messageError("Directory generation not successful.", nullptr, nullptr);
    }

    return 1;
}

bool PrintVerilogVisitor::_isSupportDeclaration(Declaration *d)
{
    if (dynamic_cast<Variable *>(d) != nullptr) {
        Variable *v       = static_cast<Variable *>(d);
        std::string vName = v->getName();
        if (_startsWith(vName, "old_") || _endsWith(vName, "_sig_var"))
            return true;
    }
    if (dynamic_cast<Procedure *>(d) != nullptr) {
        Procedure *p      = static_cast<Procedure *>(d);
        std::string pName = p->getName();
        if (_startsWith(pName, "hif_cone_"))
            return true;
    }

    return false;
}

bool PrintVerilogVisitor::_startsWith(std::string str, std::string target)
{
    std::size_t pos = str.find(target);
    if (pos == std::string::npos)
        return false;
    return (pos == 0);
}

bool PrintVerilogVisitor::_endsWith(std::string str, std::string target)
{
    std::size_t pos = str.find(target);
    if (pos == std::string::npos)
        return false;
    return (pos == (str.length() - target.length()));
}

template <typename T> void PrintVerilogVisitor::_printList(BList<T> &list, const char separator, const bool needNewLine)
{
    BList<Object> *o = reinterpret_cast<BList<Object> *>(&list);
    _printList(*o, std::string() + separator, needNewLine);
}

template <typename T>
void PrintVerilogVisitor::_printList(BList<T> &list, const std::string &separator, const bool needNewLine)
{
    BList<Object> *o = reinterpret_cast<BList<Object> *>(&list);
    _printList(*o, separator, needNewLine);
}

void PrintVerilogVisitor::_printList(BList<Object> &list, const std::string &separator, const bool needNewLine)
{
    if (list.empty())
        return;

    for (BList<Object>::iterator it(list.begin()); it != list.end(); ++it) {
        if (it != list.begin()) {
            if (separator != " ")
                *(_outstream) << separator << " ";
            else if (!needNewLine)
                *(_outstream) << " ";

            if (needNewLine && _outstream != nullptr)
                _outstream->newLine();
        }

        (*it)->acceptVisitor(*this);
    }
}

void PrintVerilogVisitor::_setRealRange(Range *o)
{
    if (dynamic_cast<Real *>(o->getParent()) != nullptr) {
        _isRealRange = true;
        return;
    }

    DataDeclaration *dd = dynamic_cast<DataDeclaration *>(o->getParent());
    if (dd == nullptr) {
        _isRealRange = false;
        return;
    }

    Real *rt     = dynamic_cast<Real *>(hif::semantics::getBaseType(dd->getType(), false, _sem));
    _isRealRange = (rt != nullptr);
}

void PrintVerilogVisitor::_printValueInstance(Value *v)
{
    if (v == nullptr)
        return;

    const bool needParen = (dynamic_cast<Expression *>(v) != nullptr);
    if (needParen)
        *(_outstream) << "(";
    v->acceptVisitor(*this);
    if (needParen)
        *(_outstream) << ")";

    bool printDot = false;

    Instance *inst = dynamic_cast<Instance *>(v);
    if (inst != nullptr) {
        Library *lib = dynamic_cast<Library *>(inst->getReferencedType());
        if (lib != nullptr && lib->isStandard())
            return;

        printDot = true;
    }

    if (printDot)
        *(_outstream) << ".";
    else
        *(_outstream) << "'";
}

void PrintVerilogVisitor::_printTypeInstance(ReferencedType *v)
{
    if (v == nullptr)
        return;

    Library *lib = dynamic_cast<Library *>(v);
    if (lib != nullptr && lib->isStandard())
        return;

    v->acceptVisitor(*this);

    *(_outstream) << ".";
}

void PrintVerilogVisitor::_printPortDirection(PortDirection dir)
{
    switch (dir) {
    case dir_in:
        *(_outstream) << "in";
        break;
    case dir_out:
        *(_outstream) << "out";
        break;
    case dir_inout:
        *(_outstream) << "inout";
        break;
    case dir_none:
    default:
        messageAssert(dir != dir_none, "Not valid port direction", nullptr, _sem);
    }
}

void PrintVerilogVisitor::_printLibraries(BList<Library> &libraries)
{
    bool libDecl = false;
    string libraryName;

    // First print the eventual IEEE libraries
    for (BList<Library>::iterator it = libraries.begin(); it != libraries.end(); ++it) {
        Library *lib = *it;
        libraryName  = lib->getName();
        if (lib->isStandard())
            continue;

        if (libraryName == "standard")
            continue;

        if (!libDecl) {
            Library *terminal = dynamic_cast<Library *>(hif::getTerminalInstance(lib));
            if (terminal->getName() == "ieee") {
                *(_outstream) << "library IEEE;";
                _outstream->newLine();
                libDecl = true;
            }
        }

        *(_outstream) << "use ";
        lib->acceptVisitor(*this);
        *(_outstream) << ".all;";
        _outstream->newLine();
    }
}

bool PrintVerilogVisitor::_printFileVariable(Variable *o)
{
    File *f = dynamic_cast<File *>(hif::semantics::getBaseType(o->getType(), false, _sem));
    if (f == nullptr)
        return false;

    FunctionCall *fc = nullptr;
    if (o->getValue() != nullptr) {
        fc = dynamic_cast<FunctionCall *>(o->getValue());
        if (fc == nullptr || fc->getName() != "file_open" || fc->parameterAssigns.size() != 2) {
            return false;
        }
    }

    *(_outstream) << "file " << o->getName() << ": ";

    o->getType()->acceptVisitor(*this);

    if (o->getValue() != nullptr) {
        messageAssert(fc != nullptr, "Unexpected case", nullptr, nullptr);
        ParameterAssign *p1 = fc->parameterAssigns.front();
        ParameterAssign *p2 = fc->parameterAssigns.back();

        *(_outstream) << " open ";
        p2->getValue()->acceptVisitor(*this);
        *(_outstream) << " is ";
        p1->getValue()->acceptVisitor(*this);
    }

    return true;
}

bool PrintVerilogVisitor::_printAssertStatement(ProcedureCall *o)
{
    // void ASSERT(bool CONDITION, string REPORT = "", severity_level LEVEL = NOTE)

    if (o->getName() != "assert")
        return false;
    const BList<ParameterAssign>::size_t size = o->parameterAssigns.size();
    if (size < 1 || size > 3)
        return false;

    *(_outstream) << "assert ";

    ParameterAssign *p1 = o->parameterAssigns.at(0);
    ParameterAssign *p2 = nullptr;
    if (size > 1)
        p2 = o->parameterAssigns.at(1);
    ParameterAssign *p3 = nullptr;
    if (size > 2)
        p3 = o->parameterAssigns.at(2);

    p1->getValue()->acceptVisitor(*this);

    if (p2 != nullptr) {
        *(_outstream) << " " << p2->getName() << " ";
        p2->getValue()->acceptVisitor(*this);
    }

    if (p3 != nullptr) {
        *(_outstream) << " " << p3->getName() << " ";
        p3->getValue()->acceptVisitor(*this);
    }

    return true;
}

// return true if the difference between a and b is within epsilon percent of the larger of a and b
bool PrintVerilogVisitor::_approximatelyEqual(double a, double b, double epsilon)
{
    return (std::abs(a - b) <= (std::max(std::abs(a), std::abs(b)) * epsilon));
}

void PrintVerilogVisitor::processDeclarations(Contents &o)
{
    if (!o.declarations.empty()) {
        for (const auto &declaration : o.declarations) {
            if (this->_isSupportDeclaration(declaration))
                continue;
            if (!_ams_enabled) {
                this->processNonAMSDeclaration(declaration);
            } else {
                this->processAMSDeclaration(declaration);
            }
        }
    }
}

void PrintVerilogVisitor::processNonAMSDeclaration(Declaration *decl)
{
    if (auto *v = dynamic_cast<Variable *>(decl)) {
        this->printWire(v);
    } else if (auto *s = dynamic_cast<Signal *>(decl)) {
        this->printReg(s);
    }
}

void PrintVerilogVisitor::printWire(Variable *v)
{
    *(_outstream) << "\twire ";
    this->printBitwidth(v->getType());
    *(_outstream) << v->getName();
    this->printValue(v->getValue());
    *(_outstream) << ";\n";
}

void PrintVerilogVisitor::printReg(Signal *s)
{
    *(_outstream) << "\treg ";
    this->printBitwidth(s->getType());
    *(_outstream) << s->getName();
    this->printValue(s->getValue());
    *(_outstream) << ";\n";
}

void PrintVerilogVisitor::printBitwidth(Type *type)
{
    unsigned long long bw = hif::semantics::typeGetSpanBitwidth(type, _sem);
    if (bw != 1) {
        *(_outstream) << "[" << (bw - 1) << ":0] ";
    }
}

void PrintVerilogVisitor::printValue(Value *value)
{
    if (auto *bv = dynamic_cast<BitValue *>(value)) {
        if (bv->getValue() != BitConstant::bit_x)
            *(_outstream) << " = " << bv->toString();
    } else if (auto *bvv = dynamic_cast<BitvectorValue *>(value)) {
        if (!bvv->isX())
            *(_outstream) << " = " << bvv->getValue();
    }
}

void PrintVerilogVisitor::processAMSDeclaration(Declaration *decl)
{
    if (auto *a = dynamic_cast<Alias *>(decl)) {
        printAlias(a);
    } else if (auto *v = dynamic_cast<Variable *>(decl)) {
        printAMSVariable(v);
    }
}

void PrintVerilogVisitor::printAlias(Alias *a)
{
    _outstream->indent();
    *(_outstream) << "branch";
    a->getValue()->acceptVisitor(*this);
    *(_outstream) << " " << a->getName() << ";\n";
    _outstream->unindent();
}

void PrintVerilogVisitor::printAMSVariable(Variable *v)
{
    std::string pName = v->getName();
    if (!pName.empty()) {
        _outstream->indent();
        if (auto *portType = dynamic_cast<ViewReference *>(v->getType())) {
            printAMSViewReference(portType, pName);
        } else if (auto *portTypeRef = dynamic_cast<TypeReference *>(v->getType())) {
            printAMSTypeReference(portTypeRef, pName);
        }
        _outstream->unindent();
    }
}

void PrintVerilogVisitor::printAMSViewReference(ViewReference *portType, const std::string &pName)
{
    std::string prefix("hif_verilog_");
    std::string currentType = portType->getDesignUnit();
    if (!currentType.compare(0, prefix.size(), prefix)) {
        *(_outstream) << currentType.substr(prefix.length()) + " " + pName << ";\n";
    }
}

void PrintVerilogVisitor::printAMSTypeReference(TypeReference *portTypeRef, const std::string &pName)
{
    std::string prefix("hif_verilog_");
    std::string currentType = portTypeRef->getName();
    if (!currentType.compare(0, prefix.size(), prefix)) {
        *(_outstream) << currentType.substr(prefix.length()) + " " + pName << ";\n";
    }
    if (auto *portTypeRef2 = portTypeRef->templateParameterAssigns.findByName("T")) {
        auto viewRef2 = dynamic_cast<hif::TypeTPAssign *>(portTypeRef2)->getType();
        auto typeRef2 = dynamic_cast<hif::ViewReference *>(viewRef2)->getDesignUnit();
        assert(!typeRef2.empty());
        currentType = typeRef2;
        if (!currentType.compare(0, prefix.size(), prefix)) {
            *(_outstream) << currentType.substr(prefix.length()) + " " + pName << ";\n";
        }
    }
}

void PrintVerilogVisitor::processStateTables(Contents &o)
{
    if (!o.stateTables.empty() && _ams_enabled) {
        for (const auto &state_table : o.stateTables) {
            _outstream->indent();
            _outstream->newLine();
            *(_outstream) << "analog begin\n";
            auto process = state_table->states.findByName("process");
            if (process) {
                _outstream->indent();
                process->acceptVisitor(*this);
                _outstream->unindent();
            }
            *(_outstream) << "\nend";
            _outstream->unindent();
        }
    }
}
