/// @file VHDLPrinter.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2vhdl/VHDLPrinter.hpp"
#include "hif2vhdl/PrintMethods.hpp"

using std::endl;
using std::string;
using std::vector;
using namespace hif;
using namespace hif::semantics;

VHDLPrinter::VHDLPrinter(const std::string &outDir)
    : _sem(hif::semantics::VHDLSemantics::getInstance())
    , _outDir(outDir)
    , _outstream(nullptr)
    , _currentDesignUnitName("")
    , _currentViewName("")
    , _currentSystem(nullptr)
    , _currentContents(nullptr)
    , _printedComponents()
    , _isPrintComponents(false)
    , _isRealRange(false)
    , _isPrintWithCondition(false)
    , _isPrintingLibDefDecls(false)
    , _isSubProgramBody(false)
{
    // ntd
}

VHDLPrinter::~VHDLPrinter()
{
    // ntd
}

int VHDLPrinter::visitAggregate(Aggregate &o)
{
    if (o.alts.empty() && o.getOthers() == nullptr)
        return 0;
    *(_outstream) << "( ";

    _printList(o.alts, ',', false);

    if (o.getOthers() != nullptr) {
        if (!o.alts.empty())
            *(_outstream) << ", ";
        *(_outstream) << "others => ";
        o.getOthers()->acceptVisitor(*this);
    }

    *(_outstream) << " )";

    return 0;
}

int VHDLPrinter::visitAggregateAlt(AggregateAlt &o)
{
    o.getValue()->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitAlias(Alias &o) { messageError("Alias is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitArray(Array &o)
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

int VHDLPrinter::visitAssign(Assign &o)
{
    _printComment(&o);

    // handle With assign
    if (dynamic_cast<With *>(o.getRightHandSide()) != nullptr) {
        _isPrintWithCondition = true;
        o.getRightHandSide()->acceptVisitor(*this);
        _isPrintWithCondition = false;

        _outstream->newLine();
        _outstream->indent();
    }

    o.getLeftHandSide()->acceptVisitor(*this);

    // Get the declaration type of the target
    Identifier *id = dynamic_cast<Identifier *>(hif::getTerminalPrefix(o.getLeftHandSide()));
    messageAssert(id != nullptr, "Unexpected target", &o, _sem);

    Declaration *dd = getDeclaration(id, _sem);

    // if the target is a variable put := otherwise put <=
    if (dynamic_cast<Variable *>(dd)) {
        *(_outstream) << " := ";
    } else {
        *(_outstream) << " <= ";
    }

    o.getRightHandSide()->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitBit(Bit &o)
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

int VHDLPrinter::visitBitValue(BitValue &o)
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

int VHDLPrinter::visitBitvector(Bitvector &o)
{
    if (o.isLogic()) {
        if (o.isResolved()) {
            *(_outstream) << "std_logic_vector";
        } else {
            *(_outstream) << "std_ulogic_vector";
        }
    } else {
        *(_outstream) << "bit_vector";
    }

    if (dynamic_cast<Cast *>(o.getParent()) != nullptr)
        return 0;
    if (dynamic_cast<Function *>(o.getParent()) != nullptr)
        return 0;

    if (o.getSpan() != nullptr) {
        *(_outstream) << "( ";
        o.getSpan()->acceptVisitor(*this);
        *(_outstream) << " )";
    }

    return 0;
}

int VHDLPrinter::visitBitvectorValue(BitvectorValue &o)
{
    // Actually, it is not possible to cast a const...
    // so it is an error to have a non-bit value type.
    //const bool isSigned = dynamic_cast<Signed *>(o.getSyntacticType()) != nullptr;
    //const bool isUnsigned = dynamic_cast<Unsigned *>(o.getSyntacticType()) != nullptr;

    //if (isSigned) *(_outstream) << "signed(";
    //else if (isUnsigned) *(_outstream) << "unsigned(";

    *(_outstream) << "\"";
    *(_outstream) << o.getValue();
    *(_outstream) << "\"";

    //if (isSigned || isUnsigned) *(_outstream) << ")";

    return 0;
}

int VHDLPrinter::visitBool(Bool & /*o*/)
{
    *(_outstream) << "boolean";

    return 0;
}

int VHDLPrinter::visitBoolValue(BoolValue &o)
{
    if (o.getValue())
        *(_outstream) << "true";
    else
        *(_outstream) << "false";

    return 0;
}

int VHDLPrinter::visitBreak(Break &o) { messageError("Break is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitCast(Cast &o)
{
    // check null pointer
    if (dynamic_cast<Pointer *>(o.getType()) != nullptr && dynamic_cast<IntValue *>(o.getValue()) != nullptr &&
        static_cast<IntValue *>(o.getValue())->getValue() == 0) {
        *(_outstream) << "null";
        return 0;
    }

    o.getType()->acceptVisitor(*this);
    *(_outstream) << "(";
    o.getValue()->acceptVisitor(*this);
    *(_outstream) << ")";
    return 0;
}

int VHDLPrinter::visitChar(Char & /*o*/)
{
    *(_outstream) << "character";

    return 0;
}

int VHDLPrinter::visitCharValue(CharValue &o)
{
    if (o.getValue() == '\0') {
        *(_outstream) << "NUL";
    } else {
        *(_outstream) << "'" << o.getValue() << "'";
    }

    return 0;
}

int VHDLPrinter::visitConst(Const &o)
{
    _printComment(&o);

    // constant name : type := initVal;
    *(_outstream) << "constant " << o.getName() << ": ";
    o.getType()->acceptVisitor(*this);

    if (o.getRange() != nullptr) {
        *(_outstream) << " range ";

        o.getRange()->acceptVisitor(*this);
    }

    *(_outstream) << " := ";
    o.getValue()->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitContents(Contents &o)
{
    // Inside libdefs just printing the hif::entity as component.
    if (_isPrintingLibDefDecls)
        return 0;
    _printComment(&o);

    _currentContents = &o;

    *(_outstream) << "ARCHITECTURE " << _currentViewName << " OF ";
    *(_outstream) << _currentDesignUnitName << " IS ";

    _outstream->newLine();
    _outstream->newLine();
    _outstream->indent();

    // Print the component declarations
    if (!o.instances.empty()) {
        _isPrintComponents = true;
        // Instances print ";" themselves
        _printList(o.instances, " ", true);
        _isPrintComponents = false;

        _outstream->newLine();
    }

    // Print some Contents declarations
    if (!o.declarations.empty()) {
        _printList(o.declarations, ";", true);
        *_outstream << ";";
        _outstream->newLine();
    }

    _outstream->unindent();

    _outstream->newLine();
    *(_outstream) << "BEGIN";
    _outstream->newLine();
    _outstream->newLine();

    _outstream->indent();

    // Print Instances and binding
    if (!o.instances.empty()) {
        _printList(o.instances, ";", true);
        *_outstream << ";";
        _outstream->newLine();
    }

    // Print Global Actions
    GlobalAction *glob = o.getGlobalAction();
    if (glob != nullptr) {
        glob->acceptVisitor(*this);
    }

    // Print StateTables
    if (!o.stateTables.empty()) {
        _printList(o.stateTables, ";\n", true);
        *_outstream << ";\n";
        _outstream->newLine();
    }

    _outstream->unindent();
    *(_outstream) << "END " << _currentViewName;

    _currentContents = nullptr;

    return 0;
}

int VHDLPrinter::visitContinue(Continue &o) { messageError("Continue is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitDesignUnit(DesignUnit &o)
{
    _printComment(&o);

    string designUnitName(o.getName());
    _initializeOutstream(designUnitName, "");

    _currentDesignUnitName = designUnitName;

    string info("Generating VHDL code for unit ");
    info += designUnitName + ".";
    messageInfo(info);

    messageAssert(o.views.size() == 1, "Not supported more than one view", &o, _sem);
    GuideVisitor::visitDesignUnit(o);

    *_outstream << ";" << std::endl;

    return 0;
}

int VHDLPrinter::visitEnum(Enum &o)
{
    *(_outstream) << "(";
    _printList(o.values, ',', false);
    *(_outstream) << ")";

    return 0;
}

int VHDLPrinter::visitEnumValue(EnumValue &o)
{
    *(_outstream) << o.getName();

    return 0;
}

int VHDLPrinter::visitExpression(Expression &o)
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
        *(_outstream) << "not";
        break;
    case op_or:
        *(_outstream) << "or";
        break;
    case op_xor:
        *(_outstream) << "xor";
        break;
    case op_and:
        *(_outstream) << "and";
        break;

    // Binary (bitwise) operators
    case op_bnot:
        *(_outstream) << "not";
        break;
    case op_bor:
        *(_outstream) << "or";
        break;
    case op_bxor:
        *(_outstream) << "xor";
        break;
    case op_band:
        *(_outstream) << "and";
        break;
    case op_sll:
        *(_outstream) << "sll";
        break;
    case op_srl:
        *(_outstream) << "srl";
        break;
    case op_rol:
        *(_outstream) << "rol";
        break;
    case op_ror:
        *(_outstream) << "ror";
        break;

    // Concatenation operator
    case op_concat:
        *(_outstream) << "&";
        break;

    // Equality operators
    case op_eq:
    case op_case_eq:
        *(_outstream) << "=";
        break;
    case op_neq:
    case op_case_neq:
        *(_outstream) << "/=";
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
        *(_outstream) << "rem";
        break;
    /// \TODO
    case op_pow:
        *(_outstream) << "**";
        break;
    case op_abs:
        *(_outstream) << "abs";
        break;
    case op_ref:
        messageError("Operator ref does not exist in VHDL.", &o, _sem);
    case op_deref:
        messageError("Operator deref does not exist in VHDL.", &o, _sem);
    case op_sla:
        *(_outstream) << "sla";
        break;
    case op_sra:
        *(_outstream) << "sra";
        break;
    case op_mod:
        *(_outstream) << "mod";
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

int VHDLPrinter::visitFunctionCall(FunctionCall &o)
{
    _printComment(&o);

    // handle attributes
    _printValueInstance(o.getInstance());

    *(_outstream) << o.getName();

    // Print parameters
    if (!o.parameterAssigns.empty()) {
        *(_outstream) << "(";
        _printList(o.parameterAssigns, ',', false);
        *(_outstream) << ")";
    }

    return 0;
}

int VHDLPrinter::visitField(Field &o)
{
    *(_outstream) << o.getName();

    *(_outstream) << ": ";
    o.getType()->acceptVisitor(*this);

    if (o.getValue() != nullptr) {
        *(_outstream) << " := ";
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

int VHDLPrinter::visitFieldReference(FieldReference &o)
{
    if (dynamic_cast<Instance *>(o.getPrefix()) != nullptr) {
        Instance *inst = static_cast<Instance *>(o.getPrefix());
        messageAssert(
            dynamic_cast<Library *>(inst->getReferencedType()) != nullptr, "Unsupported fieldreference.", &o, _sem);
    } else {
        o.getPrefix()->acceptVisitor(*this);
        *(_outstream) << ".";
    }

    *(_outstream) << o.getName();

    return 0;
}

int VHDLPrinter::visitFile(File &o) { messageError("File is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitFor(For &o)
{
    _printComment(&o);
    if (o.getName() != NameTable::getInstance()->none()) {
        *(_outstream) << o.getName() << ": ";
    }

    Range *cond     = dynamic_cast<Range *>(o.getCondition());
    Variable *index = dynamic_cast<Variable *>(o.initDeclarations.front());
    messageAssert(
        o.initDeclarations.size() == 1 && index != nullptr && o.initValues.empty() && cond != nullptr &&
            o.stepActions.empty(),
        "Unsupported for loop.", &o, _sem);

    (*_outstream) << "FOR " << index->getName() << " in ";
    cond->acceptVisitor(*this);
    (*_outstream) << " LOOP" << std::endl;
    _outstream->indent();
    _printList(o.forActions, ';', true);
    if (!o.forActions.empty())
        (*_outstream) << ";";
    _outstream->unindent();
    (*_outstream) << std::endl << "END LOOP";

    return 0;
}

int VHDLPrinter::visitForGenerate(ForGenerate &o) { messageError("ForGenerate is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitFunction(Function &o)
{
    _printComment(&o);

    *(_outstream) << "FUNCTION ";
    *(_outstream) << o.getName();

    if (!o.parameters.empty()) {
        *(_outstream) << " (";
        _printList(o.parameters, ';', false);
        *(_outstream) << " )";
    }
    // Return type
    *(_outstream) << " RETURN ";
    o.getType()->acceptVisitor(*this);

    if (!_isPrintingLibDefDecls) {
        *(_outstream) << " IS" << std::endl;
        const bool restore = _isSubProgramBody;
        _isSubProgramBody  = true;
        o.getStateTable()->acceptVisitor(*this);
        _isSubProgramBody = restore;
    }

    return 0;
}

int VHDLPrinter::visitGlobalAction(GlobalAction &o)
{
    _printComment(&o);

    if (!o.actions.empty()) {
        _printList(o.actions, ";\n", true);
        *(_outstream) << ";\n";
        _outstream->newLine();
    }

    return 0;
}

int VHDLPrinter::visitEntity(Entity &o)
{
    _printComment(&o);

    DesignUnit *du = hif::getNearestParent<DesignUnit>(&o);
    messageAssert(du != nullptr, "Design unit not found", &o, _sem);

    View *view = dynamic_cast<View *>(o.getParent());
    messageAssert(view != nullptr, "View not found.", &o, _sem);

    // Inside libdefs just printing the hif::entity as component.
    if (_isPrintingLibDefDecls) {
        *(_outstream) << "COMPONENT " << du->getName() << endl;
    } else {
        *(_outstream) << "ENTITY " << du->getName() << " IS" << endl;
    }
    _outstream->indent();

    if (!view->templateParameters.empty()) {
        *(_outstream) << "GENERIC(" << endl;
        _outstream->indent();
        _printList(view->templateParameters, ';', true);
        _outstream->unindent();
        *(_outstream) << std::endl << ");" << std::endl;
    }

    if (!o.ports.empty()) {
        *(_outstream) << "PORT(" << endl;
        _outstream->indent();

        _printList(o.ports, ';', true);

        _outstream->newLine();
        _outstream->unindent();
        *(_outstream) << ");" << endl;
    }

    _outstream->unindent();
    if (_isPrintingLibDefDecls) {
        *(_outstream) << "END COMPONENT " << du->getName();
    } else {
        *(_outstream) << "END " << du->getName();
    }

    return 0;
}

int VHDLPrinter::visitIdentifier(Identifier &o)
{
    // Print identifier name.
    *(_outstream) << o.getName();

    return 0;
}

int VHDLPrinter::visitIf(If &o)
{
    _printComment(&o);

    for (BList<IfAlt>::iterator it = o.alts.begin(); it != o.alts.end(); ++it) {
        if (it != o.alts.begin()) {
            *(_outstream) << "\nELS";
        }
        (*it)->acceptVisitor(*this);
    }

    if (!o.defaults.empty()) {
        *(_outstream) << "\nELSE";
        _outstream->newLine();
        _outstream->indent();
        _printList(o.defaults, ';', true);
        *(_outstream) << ";";
        _outstream->unindent();
    }

    *(_outstream) << std::endl << "END IF";

    return 0;
}

int VHDLPrinter::visitIfAlt(IfAlt &o)
{
    _printComment(&o);

    *(_outstream) << "IF ";
    o.getCondition()->acceptVisitor(*this);
    *(_outstream) << " THEN";
    _outstream->newLine();

    if (!o.actions.empty()) {
        _outstream->indent();
        _printList(o.actions, ';', true);
        *(_outstream) << ";";
        _outstream->unindent();
    }

    return 0;
}

int VHDLPrinter::visitIfGenerate(IfGenerate &o) { messageError("IfGenerate is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitInstance(Instance &o)
{
    ViewReference *vr = dynamic_cast<ViewReference *>(o.getReferencedType());
    Library *lib      = dynamic_cast<Library *>(o.getReferencedType());

    if (lib != nullptr) {
        lib->acceptVisitor(*this);
        return 0;
    }

    // Printing components:
    if (_isPrintComponents) {
        View *parentView = dynamic_cast<View *>(_currentContents->getParent());
        messageAssert(parentView != nullptr, "View not found", _currentContents, _sem);

        ViewReference::DeclarationType *view = hif::semantics::getDeclaration(vr, _sem);
        messageAssert(view != nullptr, "Declaration not found", vr, _sem);

        if (_printedComponents[parentView].find(view) != _printedComponents[parentView].end())
            return 0;
        _printedComponents[parentView].insert(view);

        const bool restore     = _isPrintingLibDefDecls;
        _isPrintingLibDefDecls = true;
        view->getEntity()->acceptVisitor(*this);
        _isPrintingLibDefDecls = restore;

        // Instances print ";" themselves
        *(_outstream) << ";";

        return 0;
    }

    // Print Port Map
    // Print the instance
    *(_outstream) << o.getName(); // Instance name
    *(_outstream) << ": ";
    *(_outstream) << vr->getDesignUnit() << endl; // DesignUnit name
    _outstream->indent();

    vr->acceptVisitor(*this);

    if (!o.portAssigns.empty()) {
        *(_outstream) << "PORT MAP (";
        _outstream->newLine();
        _outstream->indent();

        // Print binding
        _printList(o.portAssigns, ',', true);

        _outstream->unindent();
        *(_outstream) << ")";
    }
    _outstream->unindent();
    return 0;
}

int VHDLPrinter::visitInt(Int &o)
{
    *(_outstream) << (o.isSigned() ? "integer" : "natural");

    return 0;
}

int VHDLPrinter::visitIntValue(IntValue &o)
{
    *(_outstream) << o.getValue();
    if (_isRealRange)
        *(_outstream) << ".0";

    return 0;
}

int VHDLPrinter::visitLibraryDef(LibraryDef &o)
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

int VHDLPrinter::visitLibrary(Library &o)
{
    if (o.isStandard())
        return 0;

    _printTypeInstance(o.getInstance());

    if (o.getInstance() == nullptr && !o.isStandard() && !o.isSystem()) {
        *(_outstream) << "work.";
    }

    *(_outstream) << o.getName();

    return 0;
}

int VHDLPrinter::visitMember(Member &o)
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

int VHDLPrinter::visitNull(Null & /*o*/)
{
    *(_outstream) << "nullptr";
    return 0;
}

int VHDLPrinter::visitTransition(Transition &o) { messageError("Transition is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitParameterAssign(ParameterAssign &o)
{
    o.getValue()->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitParameter(Parameter &o)
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

int VHDLPrinter::visitProcedureCall(ProcedureCall &o)
{
    _printComment(&o);

    if (_printAssertStatement(&o))
        return 0;

    // handle attributes
    _printValueInstance(o.getInstance());

    *(_outstream) << o.getName();

    // Print parameters
    if (!o.parameterAssigns.empty()) {
        *(_outstream) << "(";
        _printList(o.parameterAssigns, ',', false);
        *(_outstream) << ")";
    }

    return 0;
}

int VHDLPrinter::visitPointer(Pointer &o)
{
    *(_outstream) << "access ";
    o.getType()->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitPortAssign(PortAssign &o)
{
    *(_outstream) << o.getName() << " => ";
    o.getValue()->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitPort(Port &o)
{
    _printComment(&o);

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

int VHDLPrinter::visitProcedure(Procedure &o)
{
    _printComment(&o);

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

int VHDLPrinter::visitRange(Range &o)
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

int VHDLPrinter::visitReal(Real & /*o*/)
{
    *(_outstream) << "real";
    return 0;
}

int VHDLPrinter::visitRealValue(RealValue &o)
{
    *(_outstream) << std::showpoint;
    *(_outstream) << o.getValue();

    return 0;
}

int VHDLPrinter::visitRecord(Record &o)
{
    *(_outstream) << "RECORD" << endl;
    _outstream->indent();
    _printList(o.fields, ';', true);
    _outstream->unindent();
    *(_outstream) << "END RECORD" << endl;
    return 0;
}

int VHDLPrinter::visitRecordValue(RecordValue &o)
{
    *(_outstream) << "( ";

    _printList(o.alts, ',', true);

    *(_outstream) << " )";

    return 0;
}

int VHDLPrinter::visitRecordValueAlt(RecordValueAlt &o)
{
    *(_outstream) << o.getName() << " <= ";
    o.getValue()->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitReference(Reference &o) { messageError("Reference is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitReturn(Return &o)
{
    *(_outstream) << "RETURN";
    if (o.getValue() != nullptr) {
        *(_outstream) << " ";
        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

int VHDLPrinter::visitSignal(Signal &o)
{
    _printComment(&o);

    // signal identifier : subtype_indication [ := expression ];
    // E.g.: signal name: integer range 7 downto 0;
    *(_outstream) << "signal " << o.getName() << ": ";

    o.getType()->acceptVisitor(*this);

    if (o.getRange() != nullptr) {
        *(_outstream) << " range ";
        o.getRange()->acceptVisitor(*this);
    }

    if (o.getValue() != nullptr) {
        *(_outstream) << " := ";

        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

int VHDLPrinter::visitSigned(Signed &o)
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

int VHDLPrinter::visitSlice(Slice &o)
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

int VHDLPrinter::visitState(State &o)
{
    if (!o.actions.empty()) {
        _printList(o.actions, ';', true);
        *(_outstream) << ";";
    }

    return 0;
}

int VHDLPrinter::visitString(String &o)
{
    *(_outstream) << "string";
    if (o.getSpanInformation() != nullptr) {
        *(_outstream) << " ( ";
        o.getSpanInformation()->acceptVisitor(*this);
        *(_outstream) << " )";
    }

    return 0;
}

int VHDLPrinter::visitStateTable(StateTable &o)
{
    _printComment(&o);

    if (!_isSubProgramBody) {
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

    // StateTable body
    *(_outstream) << "BEGIN" << endl;
    _outstream->indent();
    _outstream->newLine();

    messageAssert(o.edges.empty(), "Not empty edges list", &o, _sem);
    messageAssert(o.states.size() == 1, "Unsupported multiple states", &o, _sem);
    o.states.front()->acceptVisitor(*this);

    _outstream->unindent();
    if (!_isSubProgramBody)
        *(_outstream) << "\n\nEND PROCESS";
    else
        *(_outstream) << "\n\nEND";

    return 0;
}

int VHDLPrinter::visitSystem(System &o)
{
    _printComment(&o);

    _currentSystem = &o;

    // Create the source directory
    string dirName = _outDir + "/src";
    _createDirectory(dirName);

    visitList(o.libraryDefs);
    visitList(o.designUnits);

    messageAssert(o.libraries.empty(), "Unsupported global libreies", nullptr, _sem);
    messageAssert(o.declarations.empty(), "Unsupported global declarations", nullptr, _sem);

    return 0;
}

int VHDLPrinter::visitSwitchAlt(SwitchAlt &o)
{
    *(_outstream) << "WHEN ";
    _printList(o.conditions, " |", false);

    *(_outstream) << " =>\n";
    _outstream->indent();
    _printList(o.actions, ';', true);
    *(_outstream) << ";";
    _outstream->unindent();

    return 0;
}

int VHDLPrinter::visitSwitch(Switch &o)
{
    _printComment(&o);

    *(_outstream) << "CASE ";
    o.getCondition()->acceptVisitor(*this);
    *(_outstream) << " IS";
    _outstream->newLine();
    _outstream->indent();

    // Cases
    if (!o.alts.empty()) {
        _printList(o.alts, ' ', true);
        _outstream->newLine();
    }

    // Default case
    if (!o.defaults.empty()) {
        *(_outstream) << "WHEN OTHERS =>\n";
        _outstream->indent();
        _printList(o.defaults, ';', true);
        *(_outstream) << ";";
        _outstream->newLine();
        _outstream->unindent();
    }

    _outstream->unindent();
    *(_outstream) << "END CASE";

    return 0;
}

int VHDLPrinter::visitStringValue(StringValue &o)
{
    if (o.getValue() == "") {
        *(_outstream) << "(others => NUL)";
    } else {
        *(_outstream) << "\"" << o.getValue() << "\"";
    }
    return 0;
}

int VHDLPrinter::visitTime(Time &o) { messageError("Time is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitTimeValue(TimeValue &o) { messageError("TimeValue is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitTypeDef(TypeDef &o)
{
    _printComment(&o);

    if (o.isOpaque()) {
        *(_outstream) << "type ";
    } else {
        *(_outstream) << "subtype ";
    }

    *(_outstream) << o.getName();
    *(_outstream) << " is ";

    // Print the type
    o.getType()->acceptVisitor(*this);

    // Print the eventual range
    if (o.getRange() != nullptr) {
        *(_outstream) << " range ";
        o.getRange()->acceptVisitor(*this);
    }

    return 0;
}

int VHDLPrinter::visitTypeReference(TypeReference &o)
{
    _printTypeInstance(o.getInstance());

    *(_outstream) << o.getName();

    return 0;
}

int VHDLPrinter::visitTypeTPAssign(TypeTPAssign &o)
{
    messageError("TypeTPAssign is not implemented yet.", &o, nullptr);
}

int VHDLPrinter::visitTypeTP(TypeTP &o) { messageError("TypeTP is not implemented yet.", &o, nullptr); }

int VHDLPrinter::visitUnsigned(Unsigned &o)
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

int VHDLPrinter::visitValueTPAssign(ValueTPAssign &o)
{
    *(_outstream) << o.getName() << " => ";

    o.getValue()->acceptVisitor(*this);
    return 0;
}

int VHDLPrinter::visitValueTP(ValueTP &o)
{
    _printComment(&o);

    // identifier : subtype_indication [ := expression ];
    // E.g.: variable name: integer range 7 downto 0;
    *(_outstream) << o.getName() << ": ";

    o.getType()->acceptVisitor(*this);

    if (o.getRange() != nullptr) {
        *(_outstream) << " ";

        o.getRange()->acceptVisitor(*this);
    }

    if (o.getValue() != nullptr) {
        *(_outstream) << " := ";

        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

int VHDLPrinter::visitVariable(Variable &o)
{
    _printComment(&o);

    if (_printFileVariable(&o))
        return 0;

    // variable identifier : subtype_indication [ := expression ];
    // E.g.: variable name: integer range 7 downto 0;
    *(_outstream) << "variable " << o.getName() << ": ";

    o.getType()->acceptVisitor(*this);

    if (o.getRange() != nullptr) {
        *(_outstream) << " range ";

        o.getRange()->acceptVisitor(*this);
    }

    if (o.getValue() != nullptr) {
        *(_outstream) << " := ";

        o.getValue()->acceptVisitor(*this);
    }

    return 0;
}

int VHDLPrinter::visitView(View &o)
{
    _printComment(&o);

    _currentViewName = o.getName();

    // Get the contents of the current view
    Contents *cnt = o.getContents();
    Entity *en    = o.getEntity();

    // Print libraries
    _printLibraries(o.libraries);

    _outstream->newLine();

    // Visit the interface
    messageAssert(en != nullptr, "Unexpected nullptr entity", &o, _sem);
    en->acceptVisitor(*this);

    if (!_isPrintingLibDefDecls)
        *_outstream << ";" << std::endl;

    _outstream->newLine();

    // Visit the contents
    if (cnt != nullptr)
        cnt->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitViewReference(ViewReference &o)
{
    if (!o.templateParameterAssigns.empty()) {
        *(_outstream) << "GENERIC MAP(" << endl;
        _outstream->indent();
        _printList(o.templateParameterAssigns, ',', true);
        _outstream->unindent();
        *(_outstream) << std::endl << ")" << std::endl;
    }

    return 0;
}

int VHDLPrinter::visitWait(Wait &o)
{
    if (o.getCondition() != nullptr || o.getRepetitions() != nullptr || o.getTime() != nullptr || o.sensitivity.empty())
        messageError("Wait is not implemented yet.", &o, nullptr);

    // wait on
    *(_outstream) << "WAIT ON ";
    _printList(o.sensitivity, ',', false);

    return 0;
}

int VHDLPrinter::visitWhen(When &o)
{
    _outstream->indent();
    _printList(o.alts, " ELSE", true);

    if (o.getDefault() != nullptr) {
        *(_outstream) << " ELSE\n";
        o.getDefault()->acceptVisitor(*this);
    }
    _outstream->unindent();
    return 0;
}

int VHDLPrinter::visitWhenAlt(WhenAlt &o)
{
    o.getValue()->acceptVisitor(*this);
    *(_outstream) << " WHEN ";
    o.getCondition()->acceptVisitor(*this);

    return 0;
}

int VHDLPrinter::visitWhile(While &o)
{
    if (o.getName() != NameTable::getInstance()->none()) {
        *(_outstream) << o.getName() << ": ";
    }

    *(_outstream) << "WHILE ";
    o.getCondition()->acceptVisitor(*this);
    *(_outstream) << " LOOP" << endl;

    _outstream->indent();
    _printList(o.actions, ";", true);
    _outstream->unindent();

    *(_outstream) << "END LOOP";
    return 0;
}

int VHDLPrinter::visitWith(With &o)
{
    if (_isPrintWithCondition) {
        *(_outstream) << "WITH ";
        o.getCondition()->acceptVisitor(*this);
        *(_outstream) << " SELECT";

        return 0;
    }

    // With assign values
    _printList(o.alts, ',', true);
    *(_outstream) << ",";

    _outstream->newLine();
    o.getDefault()->acceptVisitor(*this);
    *(_outstream) << " WHEN OTHERS";
    _outstream->unindent();

    return 0;
}

int VHDLPrinter::visitWithAlt(WithAlt &o)
{
    o.getValue()->acceptVisitor(*this);

    *(_outstream) << " WHEN ";
    _printList(o.conditions, " and", true);

    return 0;
}

int VHDLPrinter::_createDirectory(string dirName)
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

void VHDLPrinter::_initializeOutstream(string fileName, string subdirectory)
{
    if (fileName.empty()) {
        messageError("Empty file name", nullptr, nullptr);
    }

    string path = _outDir + "/src/" + subdirectory + fileName;

    if (_outstream != nullptr)
        delete _outstream;

    _outstream = new hif::backends::IndentedStream(path, "vhd");
    _outstream->setComment("--", "--", "");

    _printInitBanner();
}

void VHDLPrinter::_printComment(Object *o)
{
    if (!o->hasComments())
        return;

    _outstream->newLine();
    for (std::list<std::string>::iterator it = o->getComments().begin(); it != o->getComments().end(); ++it) {
        // A new line is necessary to not interfere with current printing.
        _outstream->setCommentMode(true);
        *(_outstream) << *it;
        _outstream->setCommentMode(false);
        _outstream->newLine();
    }
}

void VHDLPrinter::_printInitBanner()
{
    *(_outstream) << "-- @brief This file was generated by hif2vhdl.\n";
    *(_outstream) << "-- @details\n";
    *(_outstream) << "-- Generate with HIF version " << hif::application_utils::getHIFVersion() << ".\n";
    *(_outstream) << "\n";
    *(_outstream) << "\n";
}

void VHDLPrinter::_printPortDirection(PortDirection dir)
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

void VHDLPrinter::_printLibraries(BList<Library> &libraries)
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

bool VHDLPrinter::_printFileVariable(Variable *o)
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

bool VHDLPrinter::_printAssertStatement(ProcedureCall *o)
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

void VHDLPrinter::_printValueInstance(Value *v)
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

void VHDLPrinter::_printTypeInstance(ReferencedType *v)
{
    if (v == nullptr)
        return;

    Library *lib = dynamic_cast<Library *>(v);
    if (lib != nullptr && lib->isStandard())
        return;

    v->acceptVisitor(*this);

    *(_outstream) << ".";
}

template <typename T> void VHDLPrinter::_printList(BList<T> &list, const char separator, const bool needNewLine)
{
    BList<Object> *o = reinterpret_cast<BList<Object> *>(&list);
    _printList(*o, std::string() + separator, needNewLine);
}

template <typename T> void VHDLPrinter::_printList(BList<T> &list, const std::string &separator, const bool needNewLine)
{
    BList<Object> *o = reinterpret_cast<BList<Object> *>(&list);
    _printList(*o, separator, needNewLine);
}

void VHDLPrinter::_printList(BList<Object> &list, const std::string &separator, const bool needNewLine)
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

void VHDLPrinter::_setRealRange(Range *o)
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
