#include "interrupt.hpp"

#include <Exception.hpp>

Assembler::x86::Simple_Interrupt_Instruction::Simple_Interrupt_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    switch (instr)
    {
        case x86::Instruction::INT3:
            opcode = 0xCC;
            break;

        case x86::Instruction::INTO:
            if (bits == BitMode::Bits64) throw Exception::SyntaxError("instruction not supported in 64-bit mode", line, column, file.c_str());
            opcode = 0xCE;
            break;

        case x86::Instruction::INT1:
            opcode = 0xF1;
            break;

        case x86::Instruction::IRET:
            opcode = 0xCF;
            if (bits != BitMode::Bits64) break;
            [[fallthrough]];

        case x86::Instruction::IRETQ:
            if (bits != BitMode::Bits64) throw Exception::SyntaxError("instruction only supported in 64-bit mode", line, column, file.c_str());
            opcode = 0xCF;
            rex.use = true;
            rex.w = true;
            break;

        case x86::Instruction::IRETD:
            if (bits == BitMode::Bits16) use16BitPrefix = true;
            opcode = 0xCF;
            break;

        case x86::Instruction::SYSCALL:
            opcodeEscape = OpcodeEscape::TWO_BYTE;
            opcode = 0x05;
            break;

        case x86::Instruction::SYSRET:
            opcodeEscape = OpcodeEscape::TWO_BYTE;
            opcode = 0x07;
            break;

        case x86::Instruction::SYSENTER:
            opcodeEscape = OpcodeEscape::TWO_BYTE;
            opcode = 0x34;
            break;

        case x86::Instruction::SYSEXIT:
            opcodeEscape = OpcodeEscape::TWO_BYTE;
            opcode = 0x35;
            break;

        default:
            throw Exception::InternalError("Invalid simple interrupt instruction", line, column, file.c_str());
    }
}
