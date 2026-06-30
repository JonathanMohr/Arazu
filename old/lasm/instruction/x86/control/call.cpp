#include "control.hpp"

#include <Exception.hpp>

Assembler::x86::CALL_Instruction::CALL_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::CALL)
        throw Exception::InternalError("Invalid call instruction", line, column, file.c_str());

    if (operand.isRegister() || operand.isMemory())
    {
        uint64_t implicitSize;
        switch (bits)
        {
            case BitMode::Bits16: implicitSize = 16; break;
            case BitMode::Bits32: implicitSize = 32; break;
            case BitMode::Bits64: implicitSize = 64; break;
            default: throw Exception::InternalError("Invalid bits", line, column, file.c_str());
        }

        uint64_t operandSize;
        if (operand.isRegister())
            operandSize = parseRegister(operand.getRegister(), true, true);
        else
            operandSize = parseMemory(operand.getMemory(), implicitSize);

        switch (operandSize)
        {
            case 8:
                throw Exception::SemanticError("Can't call to 8-bit operands", line, column, file.c_str());

            case 16:
                if (bits == BitMode::Bits32) use16BitPrefix = true;
                if (bits == BitMode::Bits64) throw Exception::SemanticError("Can't call to 16-bit operands in 64-bit mode", line, column, file.c_str());
                break;

            case 32:
                if (bits == BitMode::Bits16) use16BitPrefix = true;
                if (bits == BitMode::Bits64) throw Exception::SemanticError("Can't call to 32-bit operands in 64 bit mode", line, column, file.c_str());
                break;

            case 64:
                if (bits == BitMode::Bits64) break;
                throw Exception::SemanticError("Can't call to 64-bit operands in 16-bit/32-bit mode", line, column, file.c_str());

            default:
                throw Exception::InternalError("Invalid operand size", line, column, file.c_str());
        }

        opcode = 0xFF;
        modrm.use = true;
        modrm.reg = 2;
    }
    else
    {
        immediate.use = true;
        immediate.immediate = operand.getImmediate();
        immediate.ripRelative = true;

        immediate.is_signed = true;

        switch (bits)
        {
            case BitMode::Bits16:
                immediate.sizeInBits = 16;
                break;

            case BitMode::Bits32: case BitMode::Bits64:
                immediate.sizeInBits = 32;
                break;
        }

        opcode = 0xE8;
    }
}
