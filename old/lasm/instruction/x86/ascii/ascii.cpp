#include "ascii.hpp"

#include <Exception.hpp>

Assembler::x86::ASCII_Instruction::ASCII_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (bits == BitMode::Bits64)
        throw Exception::SyntaxError("ASCII instruction not supported in 64-bit mode", line, column, file.c_str());

    if (instr != x86::Instruction::AAA && instr != x86::Instruction::AAS)
        throw Exception::InternalError("Invalid ascii instruction", line, column, file.c_str());

    if (instr == x86::Instruction::AAA)
        opcode = 0x37;
    else
        opcode = 0x3F;
}

Assembler::x86::ASCII_Instruction::ASCII_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (bits == BitMode::Bits64)
        throw Exception::SyntaxError("ASCII instruction not supported in 64-bit mode", line, column, file.c_str());

    if (instr != x86::Instruction::AAD && instr != x86::Instruction::AAM)
        throw Exception::InternalError("Invalid ascii instruction", line, column, file.c_str());

    if (instr == x86::Instruction::AAD)
        opcode = 0xD5;
    else
        opcode = 0xD4;

    immediate.use = true;
    immediate.sizeInBits = 8;
    immediate.immediate = imm;
}
