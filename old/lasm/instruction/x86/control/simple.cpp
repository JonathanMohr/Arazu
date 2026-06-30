#include "control.hpp"

#include <Exception.hpp>

Assembler::x86::Simple_Control_Instruction::Simple_Control_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    switch (instr)
    {
        case x86::Instruction::NOP:
            opcode = 0x90;
            break;

        case x86::Instruction::HLT:
            opcode = 0xF4;
            break;

        default:
            throw Exception::InternalError("Invalid simple control instruction", line, column, file.c_str());
    }
}
