/// @file VerilogPrinter.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2verilog/VerilogPrinter.hpp"
#include "hif2verilog/PrintMethods.hpp"

VerilogPrinter::VerilogPrinter(const std::string &outDir)
    : _sem(semantics::VHDLSemantics::getInstance())
    , _outDir(outDir)
{
    // ntd
}

VerilogPrinter::~VerilogPrinter()
{
    // ntd
}

int VerilogPrinter::visitAggregate(Aggregate &o)
{
    GuideVisitor::visitAggregate(o);
    return 0;
}

int VerilogPrinter::visitAggregateAlt(AggregateAlt &o)
{
    GuideVisitor::visitAggregateAlt(o);
    return 0;
}

int VerilogPrinter::visitAlias(Alias &o)
{
    GuideVisitor::visitAlias(o);
    return 0;
}

int VerilogPrinter::visitArray(Array &o)
{
    GuideVisitor::visitArray(o);
    return 0;
}

int VerilogPrinter::visitAssign(Assign &o)
{
    GuideVisitor::visitAssign(o);
    return 0;
}

int VerilogPrinter::visitBit(Bit &o)
{
    GuideVisitor::visitBit(o);
    return 0;
}

int VerilogPrinter::visitBitValue(BitValue &o)
{
    GuideVisitor::visitBitValue(o);
    return 0;
}

int VerilogPrinter::visitBitvector(Bitvector &o)
{
    GuideVisitor::visitBitvector(o);
    return 0;
}

int VerilogPrinter::visitBitvectorValue(BitvectorValue &o)
{
    GuideVisitor::visitBitvectorValue(o);
    return 0;
}

int VerilogPrinter::visitBool(Bool &o)
{
    GuideVisitor::visitBool(o);
    return 0;
}

int VerilogPrinter::visitBoolValue(BoolValue &o)
{
    GuideVisitor::visitBoolValue(o);
    return 0;
}

int VerilogPrinter::visitBreak(Break &o)
{
    GuideVisitor::visitBreak(o);
    return 0;
}

int VerilogPrinter::visitCast(Cast &o)
{
    GuideVisitor::visitCast(o);
    return 0;
}

int VerilogPrinter::visitChar(Char &o)
{
    GuideVisitor::visitChar(o);
    return 0;
}

int VerilogPrinter::visitCharValue(CharValue &o)
{
    GuideVisitor::visitCharValue(o);
    return 0;
}

int VerilogPrinter::visitConst(Const &o)
{
    GuideVisitor::visitConst(o);
    return 0;
}

int VerilogPrinter::visitContents(Contents &o)
{
    GuideVisitor::visitContents(o);
    return 0;
}

int VerilogPrinter::visitContinue(Continue &o)
{
    GuideVisitor::visitContinue(o);
    return 0;
}

int VerilogPrinter::visitDesignUnit(DesignUnit &o)
{
    GuideVisitor::visitDesignUnit(o);
    return 0;
}

int VerilogPrinter::visitEnum(Enum &o)
{
    GuideVisitor::visitEnum(o);
    return 0;
}

int VerilogPrinter::visitEnumValue(EnumValue &o)
{
    GuideVisitor::visitEnumValue(o);
    return 0;
}

int VerilogPrinter::visitExpression(Expression &o)
{
    GuideVisitor::visitExpression(o);
    return 0;
}

int VerilogPrinter::visitFunctionCall(FunctionCall &o)
{
    GuideVisitor::visitFunctionCall(o);
    return 0;
}

int VerilogPrinter::visitField(Field &o)
{
    GuideVisitor::visitField(o);
    return 0;
}

int VerilogPrinter::visitFieldReference(FieldReference &o)
{
    GuideVisitor::visitFieldReference(o);
    return 0;
}

int VerilogPrinter::visitFile(File &o)
{
    GuideVisitor::visitFile(o);
    return 0;
}

int VerilogPrinter::visitFor(For &o)
{
    GuideVisitor::visitFor(o);
    return 0;
}

int VerilogPrinter::visitForGenerate(ForGenerate &o)
{
    GuideVisitor::visitForGenerate(o);
    return 0;
}

int VerilogPrinter::visitFunction(Function &o)
{
    GuideVisitor::visitFunction(o);
    return 0;
}

int VerilogPrinter::visitGlobalAction(GlobalAction &o)
{
    GuideVisitor::visitGlobalAction(o);
    return 0;
}

int VerilogPrinter::visitEntity(Entity &o)
{
    GuideVisitor::visitEntity(o);
    return 0;
}

int VerilogPrinter::visitIdentifier(Identifier &o)
{
    GuideVisitor::visitIdentifier(o);
    return 0;
}

int VerilogPrinter::visitIf(If &o)
{
    GuideVisitor::visitIf(o);
    return 0;
}

int VerilogPrinter::visitIfAlt(IfAlt &o)
{
    GuideVisitor::visitIfAlt(o);
    return 0;
}

int VerilogPrinter::visitIfGenerate(IfGenerate &o)
{
    GuideVisitor::visitIfGenerate(o);
    return 0;
}

int VerilogPrinter::visitInstance(Instance &o)
{
    GuideVisitor::visitInstance(o);
    return 0;
}

int VerilogPrinter::visitInt(Int &o)
{
    GuideVisitor::visitInt(o);
    return 0;
}

int VerilogPrinter::visitIntValue(IntValue &o)
{
    GuideVisitor::visitIntValue(o);
    return 0;
}

int VerilogPrinter::visitLibraryDef(LibraryDef &o)
{
    GuideVisitor::visitLibraryDef(o);
    return 0;
}

int VerilogPrinter::visitLibrary(Library &o)
{
    GuideVisitor::visitLibrary(o);
    return 0;
}

int VerilogPrinter::visitMember(Member &o)
{
    GuideVisitor::visitMember(o);
    return 0;
}

int VerilogPrinter::visitNull(Null &o)
{
    GuideVisitor::visitNull(o);
    return 0;
}

int VerilogPrinter::visitTransition(Transition &o)
{
    GuideVisitor::visitTransition(o);
    return 0;
}

int VerilogPrinter::visitParameterAssign(ParameterAssign &o)
{
    GuideVisitor::visitParameterAssign(o);
    return 0;
}

int VerilogPrinter::visitParameter(Parameter &o)
{
    GuideVisitor::visitParameter(o);
    return 0;
}

int VerilogPrinter::visitProcedureCall(ProcedureCall &o)
{
    GuideVisitor::visitProcedureCall(o);
    return 0;
}

int VerilogPrinter::visitPointer(Pointer &o)
{
    GuideVisitor::visitPointer(o);
    return 0;
}

int VerilogPrinter::visitPortAssign(PortAssign &o)
{
    GuideVisitor::visitPortAssign(o);
    return 0;
}

int VerilogPrinter::visitPort(Port &o)
{
    GuideVisitor::visitPort(o);
    return 0;
}

int VerilogPrinter::visitProcedure(Procedure &o)
{
    GuideVisitor::visitProcedure(o);
    return 0;
}

int VerilogPrinter::visitRange(Range &o)
{
    GuideVisitor::visitRange(o);
    return 0;
}

int VerilogPrinter::visitReal(Real &o)
{
    GuideVisitor::visitReal(o);
    return 0;
}

int VerilogPrinter::visitRealValue(RealValue &o)
{
    GuideVisitor::visitRealValue(o);
    return 0;
}

int VerilogPrinter::visitRecord(Record &o)
{
    GuideVisitor::visitRecord(o);
    return 0;
}

int VerilogPrinter::visitRecordValue(RecordValue &o)
{
    GuideVisitor::visitRecordValue(o);
    return 0;
}

int VerilogPrinter::visitRecordValueAlt(RecordValueAlt &o)
{
    GuideVisitor::visitRecordValueAlt(o);
    return 0;
}

int VerilogPrinter::visitReference(Reference &o)
{
    GuideVisitor::visitReference(o);
    return 0;
}

int VerilogPrinter::visitReturn(Return &o)
{
    GuideVisitor::visitReturn(o);
    return 0;
}

int VerilogPrinter::visitSignal(Signal &o)
{
    GuideVisitor::visitSignal(o);
    return 0;
}

int VerilogPrinter::visitSigned(Signed &o)
{
    GuideVisitor::visitSigned(o);
    return 0;
}

int VerilogPrinter::visitSlice(Slice &o)
{
    GuideVisitor::visitSlice(o);
    return 0;
}

int VerilogPrinter::visitState(State &o)
{
    GuideVisitor::visitState(o);
    return 0;
}

int VerilogPrinter::visitString(String &o)
{
    GuideVisitor::visitString(o);
    return 0;
}

int VerilogPrinter::visitStateTable(StateTable &o)
{
    GuideVisitor::visitStateTable(o);
    return 0;
}

int VerilogPrinter::visitSystem(System &o)
{
    GuideVisitor::visitSystem(o);
    return 0;
}

int VerilogPrinter::visitSwitchAlt(SwitchAlt &o)
{
    GuideVisitor::visitSwitchAlt(o);
    return 0;
}

int VerilogPrinter::visitSwitch(Switch &o)
{
    GuideVisitor::visitSwitch(o);
    return 0;
}

int VerilogPrinter::visitStringValue(StringValue &o)
{
    GuideVisitor::visitStringValue(o);
    return 0;
}

int VerilogPrinter::visitTime(Time &o)
{
    GuideVisitor::visitTime(o);
    return 0;
}

int VerilogPrinter::visitTimeValue(TimeValue &o)
{
    GuideVisitor::visitTimeValue(o);
    return 0;
}

int VerilogPrinter::visitTypeDef(TypeDef &o)
{
    GuideVisitor::visitTypeDef(o);
    return 0;
}

int VerilogPrinter::visitTypeReference(TypeReference &o)
{
    GuideVisitor::visitTypeReference(o);
    return 0;
}

int VerilogPrinter::visitTypeTPAssign(TypeTPAssign &o)
{
    GuideVisitor::visitTypeTPAssign(o);
    return 0;
}

int VerilogPrinter::visitTypeTP(TypeTP &o)
{
    GuideVisitor::visitTypeTP(o);
    return 0;
}

int VerilogPrinter::visitUnsigned(Unsigned &o)
{
    GuideVisitor::visitUnsigned(o);
    return 0;
}

int VerilogPrinter::visitValueTPAssign(ValueTPAssign &o)
{
    GuideVisitor::visitValueTPAssign(o);
    return 0;
}

int VerilogPrinter::visitValueTP(ValueTP &o)
{
    GuideVisitor::visitValueTP(o);
    return 0;
}

int VerilogPrinter::visitVariable(Variable &o)
{
    GuideVisitor::visitVariable(o);
    return 0;
}

int VerilogPrinter::visitView(View &o)
{
    GuideVisitor::visitView(o);
    return 0;
}

int VerilogPrinter::visitViewReference(ViewReference &o)
{
    GuideVisitor::visitViewReference(o);
    return 0;
}

int VerilogPrinter::visitWait(Wait &o)
{
    GuideVisitor::visitWait(o);
    return 0;
}

int VerilogPrinter::visitWhen(When &o)
{
    GuideVisitor::visitWhen(o);
    return 0;
}

int VerilogPrinter::visitWhenAlt(WhenAlt &o)
{
    GuideVisitor::visitWhenAlt(o);
    return 0;
}

int VerilogPrinter::visitWhile(While &o)
{
    GuideVisitor::visitWhile(o);
    return 0;
}

int VerilogPrinter::visitWith(With &o)
{
    GuideVisitor::visitWith(o);
    return 0;
}

int VerilogPrinter::visitWithAlt(WithAlt &o)
{
    GuideVisitor::visitWithAlt(o);
    return 0;
}
