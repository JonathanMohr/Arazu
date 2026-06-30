#include "interrupt.hpp"

#include <Exception.hpp>

Assembler::x86::Normal_Interrupt_Instruction::Normal_Interrupt_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::INT)
        throw Exception::InternalError("Invalid normal interrupt instruction", line, column, file.c_str());

    opcode = 0xCD;

    immediate.use = true;
    immediate.sizeInBits = 8;
    immediate.immediate = imm;
}
