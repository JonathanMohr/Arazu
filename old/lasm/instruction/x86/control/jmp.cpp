#include "control.hpp"

#include <Exception.hpp>
#include <limits>

Assembler::x86::JMP_Instruction::JMP_Instruction(x86::Instruction instr, bool near, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f)
    : JMP_Instruction(instr, bitmode, operand, l, c, f)
{
    canOptimize = false;

    if (!near)
    {
        opcodeEscape = OpcodeEscape::NONE;

        switch (instruction)
        {
            case x86::Instruction::JMP: opcode = 0xEB; break;

            case x86::Instruction::JE: opcode = 0x74; break;
            case x86::Instruction::JNE: opcode = 0x75; break;

            case x86::Instruction::JG: opcode = 0x7F; break;
            case x86::Instruction::JGE: opcode = 0x7D; break;
            case x86::Instruction::JL: opcode = 0x7C; break;
            case x86::Instruction::JLE: opcode = 0x7E; break;

            case x86::Instruction::JA: opcode = 0x77; break;
            case x86::Instruction::JAE: opcode = 0x73; break;
            case x86::Instruction::JB: opcode = 0x72; break;
            case x86::Instruction::JBE: opcode = 0x76; break;

            case x86::Instruction::JO: opcode = 0x70; break;
            case x86::Instruction::JNO: opcode = 0x71; break;
            case x86::Instruction::JS: opcode = 0x78; break;
            case x86::Instruction::JNS: opcode = 0x79; break;

            case x86::Instruction::JP: opcode = 0x7A; break;
            case x86::Instruction::JNP: opcode = 0x7B; break;
            case x86::Instruction::JC: opcode = 0x72; break;
            case x86::Instruction::JNC: opcode = 0x73; break;

            default:
                throw Exception::InternalError("Invalid jmp instruction", line, column, file.c_str());
        }

        immediate.sizeInBits = 8;
    }
}

Assembler::x86::JMP_Instruction::JMP_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f), instruction(instr)
{
    switch (instr)
    {
        case x86::Instruction::JMP:
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
                        throw Exception::SemanticError("Can't jump to 8-bit operands", line, column, file.c_str());

                    case 16:
                        if (bits == BitMode::Bits32) use16BitPrefix = true;
                        if (bits == BitMode::Bits64) throw Exception::SemanticError("Can't jump to 16-bit operands in 64-bit mode", line, column, file.c_str());
                        break;

                    case 32:
                        if (bits == BitMode::Bits16) use16BitPrefix = true;
                        if (bits == BitMode::Bits64) throw Exception::SemanticError("Can't jump to 32-bit operands in 64 bit mode", line, column, file.c_str());
                        break;

                    case 64:
                        if (bits == BitMode::Bits64) break;
                        throw Exception::SemanticError("Can't jump to 64-bit operands in 16-bit/32-bit mode", line, column, file.c_str());

                    default:
                        throw Exception::InternalError("Invalid operand size", line, column, file.c_str());
                }

                opcode = 0xFF;
                modrm.use = true;
                modrm.reg = 4;

                break;
            }

            [[fallthrough]];
            
        case x86::Instruction::JE: case x86::Instruction::JNE:
        case x86::Instruction::JG: case x86::Instruction::JGE:
        case x86::Instruction::JL: case x86::Instruction::JLE:
        case x86::Instruction::JA: case x86::Instruction::JAE:
        case x86::Instruction::JB: case x86::Instruction::JBE:
        case x86::Instruction::JO: case x86::Instruction::JNO:
        case x86::Instruction::JS: case x86::Instruction::JNS:
        case x86::Instruction::JP: case x86::Instruction::JNP:
        case x86::Instruction::JC: case x86::Instruction::JNC:
        {
            if (!operand.isImmediate())
                throw Exception::InternalError("Only immediates are allowed for conditional jumps", line, column, file.c_str());

            immediate.use = true;
            immediate.immediate = operand.getImmediate();
            immediate.ripRelative = true;

            immediate.is_signed = true;

            canOptimize = true;

            switch (bits)
            {
                case BitMode::Bits16:
                    immediate.sizeInBits = 16;
                    break;

                case BitMode::Bits32: case BitMode::Bits64:
                    immediate.sizeInBits = 32;
                    break;
            }

            switch (instr)
            {
                case x86::Instruction::JMP: opcode = 0xE9; break;

                case x86::Instruction::JE: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x84; break;
                case x86::Instruction::JNE: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x85; break;

                case x86::Instruction::JG: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x8F; break;
                case x86::Instruction::JGE: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x8D; break;
                case x86::Instruction::JL: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x8C; break;
                case x86::Instruction::JLE: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x8E; break;

                case x86::Instruction::JA: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x87; break;
                case x86::Instruction::JAE: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x83; break;
                case x86::Instruction::JB: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x82; break;
                case x86::Instruction::JBE: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x86; break;

                case x86::Instruction::JO: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x80; break;
                case x86::Instruction::JNO: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x81; break;
                case x86::Instruction::JS: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x88; break;
                case x86::Instruction::JNS: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x89; break;

                case x86::Instruction::JP: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x8A; break;
                case x86::Instruction::JNP: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x8B; break;
                case x86::Instruction::JC: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x82; break;
                case x86::Instruction::JNC: opcodeEscape = OpcodeEscape::TWO_BYTE; opcode = 0x83; break;

                default:
                    throw Exception::InternalError("Invalid jmp instruction", line, column, file.c_str());
            }

            break;
        }

        default:
            throw Exception::InternalError("Invalid jmp instruction", line, column, file.c_str());
    }
}

bool Assembler::x86::JMP_Instruction::optimizeS()
{
    if (canOptimize && !immediate.needsRelocation)
    {
        int32_t offset;
        if (immediate.sizeInBits == 16)
            offset = static_cast<int32_t>(static_cast<int16_t>(static_cast<uint16_t>(immediate.value)));
        else
            offset = static_cast<int32_t>(static_cast<uint32_t>(immediate.value));

        if (
            offset <= static_cast<int32_t>(std::numeric_limits<int8_t>::max()) &&
            offset >= static_cast<int32_t>(std::numeric_limits<int8_t>::min())
        ) {
            canOptimize = false;

            opcodeEscape = OpcodeEscape::NONE;

            opcodeEscape = OpcodeEscape::NONE;

            switch (instruction)
            {
                case x86::Instruction::JMP: opcode = 0xEB; break;

                case x86::Instruction::JE: opcode = 0x74; break;
                case x86::Instruction::JNE: opcode = 0x75; break;

                case x86::Instruction::JG: opcode = 0x7F; break;
                case x86::Instruction::JGE: opcode = 0x7D; break;
                case x86::Instruction::JL: opcode = 0x7C; break;
                case x86::Instruction::JLE: opcode = 0x7E; break;

                case x86::Instruction::JA: opcode = 0x77; break;
                case x86::Instruction::JAE: opcode = 0x73; break;
                case x86::Instruction::JB: opcode = 0x72; break;
                case x86::Instruction::JBE: opcode = 0x76; break;

                case x86::Instruction::JO: opcode = 0x70; break;
                case x86::Instruction::JNO: opcode = 0x71; break;
                case x86::Instruction::JS: opcode = 0x78; break;
                case x86::Instruction::JNS: opcode = 0x79; break;

                case x86::Instruction::JP: opcode = 0x7A; break;
                case x86::Instruction::JNP: opcode = 0x7B; break;
                case x86::Instruction::JC: opcode = 0x72; break;
                case x86::Instruction::JNC: opcode = 0x73; break;

                default:
                    throw Exception::InternalError("Invalid jmp instruction", line, column, file.c_str());
            }

            immediate.sizeInBits = 8;

            return true;
        }
    }

    return false;
}
