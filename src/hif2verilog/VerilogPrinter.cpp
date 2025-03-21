/// @file VerilogPrinter.cpp
/// @brief
/// Copyright (c) 2024-2025, Electronic Systems Design (ESD) Group,
/// Univeristy of Verona.
/// This file is distributed under the BSD 2-Clause License.
/// See LICENSE.md for details.

#include "hif2verilog/VerilogPrinter.hpp"

#include "hif2verilog/PrintMethods.hpp"
#include <utility>

VerilogPrinter::VerilogPrinter(std::string outDir)
    : _sem(hif::semantics::VHDLSemantics::getInstance())
    , _outDir(std::move(outDir))
{
    // ntd
}

VerilogPrinter::~VerilogPrinter()
{
    // ntd
}

auto VerilogPrinter::visitAggregate(Aggregate &o) -> int
{
    GuideVisitor::visitAggregate(o);
    return 0;
}

auto VerilogPrinter::visitAggregateAlt(AggregateAlt &o) -> int
{
    GuideVisitor::visitAggregateAlt(o);
    return 0;
}

auto VerilogPrinter::visitAlias(Alias &o) -> int
{
    GuideVisitor::visitAlias(o);
    return 0;
}

auto VerilogPrinter::visitArray(Array &o) -> int
{
    GuideVisitor::visitArray(o);
    return 0;
}

auto VerilogPrinter::visitAssign(Assign &o) -> int
{
    GuideVisitor::visitAssign(o);
    return 0;
}

auto VerilogPrinter::visitBit(Bit &o) -> int
{
    GuideVisitor::visitBit(o);
    return 0;
}

auto VerilogPrinter::visitBitValue(BitValue &o) -> int
{
    GuideVisitor::visitBitValue(o);
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

auto VerilogPrinter::visitChar(char &o) -> int
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
    GuideVisitor::visitConst(o);
    return 0;
}

auto VerilogPrinter::visitContents(Contents &o) -> int
{
    GuideVisitor::visitContents(o);
    return 0;
}

auto VerilogPrinter::visitContinue(Continue &o) -> int
{
    GuideVisitor::visitContinue(o);
    return 0;
}

auto VerilogPrinter::visitDesignUnit(DesignUnit &o) -> int
{
    GuideVisitor::visitDesignUnit(o);
    return 0;
}

auto VerilogPrinter::visitEnum(Enum &o) -> int
{
    GuideVisitor::visitEnum(o);
    return 0;
}

auto VerilogPrinter::visitEnumValue(EnumValue &o) -> int
{
    GuideVisitor::visitEnumValue(o);
    return 0;
}

auto VerilogPrinter::visitExpression(Expression &o) -> int
{
    GuideVisitor::visitExpression(o);
    return 0;
}

auto VerilogPrinter::visitFunctionCall(FunctionCall &o) -> int
{
    GuideVisitor::visitFunctionCall(o);
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
    GuideVisitor::visitEntity(o);
    return 0;
}

auto VerilogPrinter::visitIdentifier(Identifier &o) -> int
{
    GuideVisitor::visitIdentifier(o);
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

auto VerilogPrinter::visitIfGenerate(IfGenerate &o) -> int
{
    GuideVisitor::visitIfGenerate(o);
    return 0;
}

auto VerilogPrinter::visitInstance(Instance &o) -> int
{
    GuideVisitor::visitInstance(o);
    return 0;
}

auto VerilogPrinter::visitInt(Int &o) -> int
{
    GuideVisitor::visitInt(o);
    return 0;
}

auto VerilogPrinter::visitIntValue(IntValue &o) -> int
{
    GuideVisitor::visitIntValue(o);
    return 0;
}

auto VerilogPrinter::visitLibraryDef(LibraryDef &o) -> int
{
    GuideVisitor::visitLibraryDef(o);
    return 0;
}

auto VerilogPrinter::visitLibrary(Library &o) -> int
{
    GuideVisitor::visitLibrary(o);
    return 0;
}

auto VerilogPrinter::visitMember(Member &o) -> int
{
    GuideVisitor::visitMember(o);
    return 0;
}

auto VerilogPrinter::visitNull(Null &o) -> int
{
    GuideVisitor::visitNull(o);
    return 0;
}

auto VerilogPrinter::visitTransition(Transition &o) -> int
{
    GuideVisitor::visitTransition(o);
    return 0;
}

auto VerilogPrinter::visitParameterAssign(ParameterAssign &o) -> int
{
    GuideVisitor::visitParameterAssign(o);
    return 0;
}

auto VerilogPrinter::visitParameter(Parameter &o) -> int
{
    GuideVisitor::visitParameter(o);
    return 0;
}

auto VerilogPrinter::visitProcedureCall(ProcedureCall &o) -> int
{
    GuideVisitor::visitProcedureCall(o);
    return 0;
}

auto VerilogPrinter::visitPointer(Pointer &o) -> int
{
    GuideVisitor::visitPointer(o);
    return 0;
}

auto VerilogPrinter::visitPortAssign(PortAssign &o) -> int
{
    GuideVisitor::visitPortAssign(o);
    return 0;
}

auto VerilogPrinter::visitPort(Port &o) -> int
{
    GuideVisitor::visitPort(o);
    return 0;
}

auto VerilogPrinter::visitProcedure(Procedure &o) -> int
{
    GuideVisitor::visitProcedure(o);
    return 0;
}

auto VerilogPrinter::visitRange(Range &o) -> int
{
    GuideVisitor::visitRange(o);
    return 0;
}

auto VerilogPrinter::visitReal(Real &o) -> int
{
    GuideVisitor::visitReal(o);
    return 0;
}

auto VerilogPrinter::visitRealValue(RealValue &o) -> int
{
    GuideVisitor::visitRealValue(o);
    return 0;
}

auto VerilogPrinter::visitRecord(Record &o) -> int
{
    GuideVisitor::visitRecord(o);
    return 0;
}

auto VerilogPrinter::visitRecordValue(RecordValue &o) -> int
{
    GuideVisitor::visitRecordValue(o);
    return 0;
}

auto VerilogPrinter::visitRecordValueAlt(RecordValueAlt &o) -> int
{
    GuideVisitor::visitRecordValueAlt(o);
    return 0;
}

auto VerilogPrinter::visitReference(Reference &o) -> int
{
    GuideVisitor::visitReference(o);
    return 0;
}

auto VerilogPrinter::visitReturn(Return &o) -> int
{
    GuideVisitor::visitReturn(o);
    return 0;
}

auto VerilogPrinter::visitSignal(Signal &o) -> int
{
    GuideVisitor::visitSignal(o);
    return 0;
}

auto VerilogPrinter::visitSigned(Signed &o) -> int
{
    GuideVisitor::visitSigned(o);
    return 0;
}

auto VerilogPrinter::visitSlice(Slice &o) -> int
{
    GuideVisitor::visitSlice(o);
    return 0;
}

auto VerilogPrinter::visitState(State &o) -> int
{
    GuideVisitor::visitState(o);
    return 0;
}

auto VerilogPrinter::visitString(String &o) -> int
{
    GuideVisitor::visitString(o);
    return 0;
}

auto VerilogPrinter::visitStateTable(StateTable &o) -> int
{
    GuideVisitor::visitStateTable(o);
    return 0;
}

auto VerilogPrinter::visitSystem(System &o) -> int
{
    GuideVisitor::visitSystem(o);
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

auto VerilogPrinter::visitTime(Time &o) -> int
{
    GuideVisitor::visitTime(o);
    return 0;
}

auto VerilogPrinter::visitTimeValue(TimeValue &o) -> int
{
    GuideVisitor::visitTimeValue(o);
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
    GuideVisitor::visitTypeTPAssign(o);
    return 0;
}

auto VerilogPrinter::visitTypeTP(TypeTP &o) -> int
{
    GuideVisitor::visitTypeTP(o);
    return 0;
}

auto VerilogPrinter::visitUnsigned(Unsigned &o) -> int
{
    GuideVisitor::visitUnsigned(o);
    return 0;
}

auto VerilogPrinter::visitValueTPAssign(ValueTPAssign &o) -> int
{
    GuideVisitor::visitValueTPAssign(o);
    return 0;
}

auto VerilogPrinter::visitValueTP(ValueTP &o) -> int
{
    GuideVisitor::visitValueTP(o);
    return 0;
}

auto VerilogPrinter::visitVariable(Variable &o) -> int
{
    GuideVisitor::visitVariable(o);
    return 0;
}

auto VerilogPrinter::visitView(View &o) -> int
{
    GuideVisitor::visitView(o);
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
    GuideVisitor::visitWhile(o);
    return 0;
}

auto VerilogPrinter::visitWith(With &o) -> int
{
    GuideVisitor::visitWith(o);
    return 0;
}

auto VerilogPrinter::visitWithAlt(WithAlt &o) -> int
{
    GuideVisitor::visitWithAlt(o);
    return 0;
}
