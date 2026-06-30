#include "control.hpp"

#include <Exception.hpp>

Assembler::x86::RET_Instruction::RET_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::RET)
        throw Exception::InternalError("Invalid ret instruction", line, column, file.c_str());

    opcode = 0xC3;
}

Assembler::x86::RET_Instruction::RET_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::RET)
        throw Exception::InternalError("Invalid ret instruction", line, column, file.c_str());

    opcode = 0xC2;

    immediate.use = true;
    immediate.sizeInBits = 16;
    immediate.immediate = imm;
}
