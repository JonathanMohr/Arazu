#include "stack.hpp"

#include <limits>
#include <Exception.hpp>

Assembler::x86::Normal_Stack_Instruction::Normal_Stack_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::PUSH && instr != x86::Instruction::POP)
        throw Exception::InternalError("Invalid normal stack instruction", line, column, file.c_str());

    if (operand.isRegister() || operand.isMemory())
    {
        uint64_t opSize;
        if (operand.isRegister())
        {
            const AsmProg::Register& reg = operand.getRegister();
            if (!isRegGPR(reg))
                throw Exception::SemanticError("Instruction only supports GPRs", line, column, file.c_str());
            
            opSize = getRegSize(reg, bits, line, column, file.c_str());
            checkSize(opSize);

            auto [regIndex, regUseREX, regSetREX] = getRegInfo(reg, line, column, file.c_str());

            if (regUseREX) rex.use = true;
            if (regSetREX) rex.b = true;

            if (instr == x86::Instruction::PUSH)
                opcode = 0x50 + regIndex;
            else // POP
                opcode = 0x58 + regIndex;
        }
        else // memory
        {
            uint64_t implicitSize;
            switch (bits)
            {
                case BitMode::Bits16: implicitSize = 16; break;
                case BitMode::Bits32: implicitSize = 32; break;
                case BitMode::Bits64: implicitSize = 64; break;
                default: throw Exception::InternalError("Invalid bits", line, column, file.c_str());
            }

            opSize = parseMemory(operand.getMemory(), implicitSize);

            if (instr == x86::Instruction::PUSH)
            {
                opcode = 0xFF;

                modrm.use = true;
                modrm.reg = 6;
            }
            else // POP
            {
                opcode = 0x8F;

                // TODO: Check
                modrm.use = true;
                modrm.reg = 0;
            }
        }

        switch (opSize)
        {
            case 16:
                if (bits != BitMode::Bits16) use16BitPrefix = true;
                break;

            case 32:
                if (bits == BitMode::Bits16) use16BitPrefix = true;
                else if (bits == BitMode::Bits64)
                    throw Exception::SyntaxError("Can't push/pop 32-bit operands in 64-bit mode", line, column, file.c_str());
                break;

            case 64:
                if (bits != BitMode::Bits64)
                    throw Exception::SyntaxError("Can only push/pop 64-bit operands in 64-bit mode", line, column, file.c_str());
                break;
                        
            case 8:
                throw Exception::SyntaxError("Can't push/pop 8-bit operands", line, column, file.c_str());

            default:
                throw Exception::InternalError("Invalid operand size", line, column, file.c_str());
        }
    }
    else
    {
        if (instr == x86::Instruction::POP)
            throw Exception::SyntaxError("Cannot pop into immediates", line, column, file.c_str());

        immediate.use = true;
        immediate.immediate = operand.getImmediate();

        immediate.is_signed = true;

        canOptimize = true;

        opcode = 0x68;

        switch (bits)
        {
            case BitMode::Bits16:
                immediate.sizeInBits = 16;
                break;

            case BitMode::Bits32: case BitMode::Bits64:
                immediate.sizeInBits = 32;
                break;

            default:
                throw Exception::InternalError("Invalid bitmode", line, column, file.c_str());
        }
    }
}

bool Assembler::x86::Normal_Stack_Instruction::optimizeS()
{
    if (canOptimize && !immediate.needsRelocation)
    {
        int32_t value;

        if (bits == BitMode::Bits16)
        {
            const int16_t val16 = static_cast<int16_t>(static_cast<uint16_t>(immediate.value));
            value = static_cast<int32_t>(val16);
        }
        else
            value = static_cast<int32_t>(static_cast<uint32_t>(immediate.value));

        if (
            value <= static_cast<int32_t>(std::numeric_limits<int8_t>::max()) &&
            value >= static_cast<int32_t>(std::numeric_limits<int8_t>::min())
        ) {
            canOptimize = false,

            opcode = 0x6A;

            immediate.sizeInBits = 8;

            return true;
        }
    }

    return false;
}
