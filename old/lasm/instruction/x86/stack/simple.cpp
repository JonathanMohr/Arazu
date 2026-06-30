#include "stack.hpp"

#include <Exception.hpp>

Assembler::x86::Simple_Stack_Instruction::Simple_Stack_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    switch (instr)
    {
        case x86::Instruction::PUSHA:
            if (bits == BitMode::Bits64) throw Exception::SyntaxError("instruction not supported in 64-bit mode", line, column, file.c_str());
            opcode = 0x60;
            break;

        case x86::Instruction::POPA:
            if (bits == BitMode::Bits64) throw Exception::SyntaxError("instruction not supported in 64-bit mode", line, column, file.c_str());
            opcode = 0x61;
            break;

        case x86::Instruction::PUSHAD:
            if (bits == BitMode::Bits64) throw Exception::SyntaxError("instruction not supported in 64-bit mode", line, column, file.c_str());
            if (bits == BitMode::Bits16) use16BitPrefix = true;
            opcode = 0x60;
            break;

        case x86::Instruction::POPAD:
            if (bits == BitMode::Bits64) throw Exception::SyntaxError("instruction not supported in 64-bit mode", line, column, file.c_str());
            if (bits == BitMode::Bits16) use16BitPrefix = true;
            opcode = 0x61;
            break;

        case x86::Instruction::PUSHF:
            opcode = 0x9C;
            break;

        case x86::Instruction::POPF:
            opcode = 0x9D;
            break;

        case x86::Instruction::PUSHFD:
            if (bits == BitMode::Bits64) throw Exception::SyntaxError("instruction not supported in 64-bit mode", line, column, file.c_str());
            if (bits == BitMode::Bits16) use16BitPrefix = true;
            opcode = 0x9C;
            break;

        case x86::Instruction::POPFD:
            if (bits == BitMode::Bits64) throw Exception::SyntaxError("instruction not supported in 64-bit mode", line, column, file.c_str());
            if (bits == BitMode::Bits16) use16BitPrefix = true;
            opcode = 0x9D;
            break;

        case x86::Instruction::PUSHFQ:
            if (bits != BitMode::Bits64) throw Exception::SyntaxError("instruction only supported in 64-bit mode", line, column, file.c_str());
            opcode = 0x9C;
            break;

        case x86::Instruction::POPFQ:
            if (bits != BitMode::Bits64) throw Exception::SyntaxError("instruction only supported in 64-bit mode", line, column, file.c_str());
            opcode = 0x9D;
            break;

        default:
            throw Exception::InternalError("Invalid simple stack instruction", line, column, file.c_str());
    }
}
