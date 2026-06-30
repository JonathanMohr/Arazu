#include "flag.hpp"

#include <Exception.hpp>

Assembler::x86::Simple_Flag_Instruction::Simple_Flag_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    switch (instr)
    {
        case x86::Instruction::CLC:
            opcode = 0xF8;
            break;

        case x86::Instruction::STC:
            opcode = 0xF9;
            break;

        case x86::Instruction::CMC:
            opcode = 0xF5;
            break;

        case x86::Instruction::CLD:
            opcode = 0xFC;
            break;

        case x86::Instruction::STD:
            opcode = 0xFD;
            break;

        case x86::Instruction::CLI:
            opcode = 0xFA;
            break;

        case x86::Instruction::STI:
            opcode = 0xFB;
            break;

        case x86::Instruction::LAHF:
            opcode = 0x9F;
            break;

        case x86::Instruction::SAHF:
            opcode = 0x9E;
            break;

        default:
            throw Exception::InternalError("Invalid simple flag instruction", line, column, file.c_str());
    }
}
