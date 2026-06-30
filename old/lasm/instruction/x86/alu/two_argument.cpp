#include "alu.hpp"

#include <Exception.hpp>
#include <limits>

Assembler::x86::Two_Argument_ALU_Instruction::Two_Argument_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& mainOperand, const AsmProg::Operand& otherOperand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f), mainOp(mainOperand)
{
    if (instr != x86::Instruction::ADD && instr != x86::Instruction::ADC &&
        instr != x86::Instruction::SUB && instr != x86::Instruction::SBB &&
        instr != x86::Instruction::CMP && instr != x86::Instruction::TEST &&
        instr != x86::Instruction::AND && instr != x86::Instruction::OR &&
        instr != x86::Instruction::XOR)
        throw Exception::InternalError("Invalid two argument ALU instruction", line, column, file.c_str());

    bool useRMFirst = true;
    bool otherIsImmediate = false;

    if (mainOperand.isRegister())
    {
        if (otherOperand.isRegister())
            useRMFirst = true;
        else if (otherOperand.isMemory())
            useRMFirst = false;
        else
            otherIsImmediate = true;
    }
    else if (mainOperand.isMemory())
    {
        if (otherOperand.isRegister())
            useRMFirst = true;
        else if (otherOperand.isMemory())
            throw Exception::SemanticError("Instruction cannot have two memory operands", line, column, file.c_str());
        else
            otherIsImmediate = true;
    }
    else
        throw Exception::SyntaxError("Cannot use immediate as main operand", line, column, file.c_str());

    if (otherIsImmediate)
    {
        if (mainOperand.isRegister())
        {
            switch (mainOperand.getRegister())
            {
                case Register::AL:
                    immediate.sizeInBits = 8;
                    accumulatorReg = true;
                    break;

                case Register::AX:
                    if (bits != BitMode::Bits16) use16BitPrefix = true;
                    immediate.sizeInBits = 16;
                    accumulatorReg = true;
                    break;

                case Register::EAX:
                    if (bits == BitMode::Bits16) use16BitPrefix = true;
                    immediate.sizeInBits = 32;
                    accumulatorReg = true;
                    break;

                case Register::RAX:
                    immediate.sizeInBits = 32;
                    rex.use = true;
                    rex.w = true;
                    accumulatorReg = true;
                    break;

                default:
                    break;
            }

        }

        immediate.use = true;
        immediate.immediate = otherOperand.getImmediate();

        if (accumulatorReg)
        {
            checkReg(mainOperand.getRegister());
            checkSize(immediate.sizeInBits);

            if (immediate.sizeInBits == 8)
            {
                switch (instr)
                {
                    case x86::Instruction::ADD: opcode = 0x04; break;
                    case x86::Instruction::OR:  opcode = 0x0C; break;
                    case x86::Instruction::ADC: opcode = 0x14; break;
                    case x86::Instruction::SBB: opcode = 0x1C; break;
                    case x86::Instruction::AND: opcode = 0x24; break;
                    case x86::Instruction::SUB: opcode = 0x2C; break;
                    case x86::Instruction::XOR: opcode = 0x34; break;
                    case x86::Instruction::CMP: opcode = 0x3C; break;
                    case x86::Instruction::TEST: opcode = 0xA8; break;
                    default:
                        throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
                }
            }
            else
            {
                if (instr != x86::Instruction::TEST) canOptimize = true;
                switch (instr)
                {
                    case x86::Instruction::ADD: opcode = 0x05; break;
                    case x86::Instruction::OR:  opcode = 0x0D; break;
                    case x86::Instruction::ADC: opcode = 0x15; break;
                    case x86::Instruction::SBB: opcode = 0x1D; break;
                    case x86::Instruction::AND: opcode = 0x25; break;
                    case x86::Instruction::SUB: opcode = 0x2D; break;
                    case x86::Instruction::XOR: opcode = 0x35; break;
                    case x86::Instruction::CMP: opcode = 0x3D; break;
                    case x86::Instruction::TEST: opcode = 0xA9; break;
                    default:
                        throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
                }
            }
        }
        else
        {
            uint64_t mainSize;
            if (mainOperand.isRegister())
                mainSize = parseRegister(mainOperand.getRegister(), true, true);
            else // Memory
                mainSize = parseMemory(mainOperand.getMemory(), AsmProg::Memory::POINTER_SIZE_NONE);

            switch (mainSize)
            {
                case 8:
                    immediate.sizeInBits = 8;
                    break;

                case 16:
                    if (bits != BitMode::Bits16) use16BitPrefix = true;
                    immediate.sizeInBits = 16;
                    break;

                case 32:
                    if (bits == BitMode::Bits16) use16BitPrefix = true;
                    immediate.sizeInBits = 32;
                    break;

                case 64:
                    immediate.sizeInBits = 32;
                    rex.use = true;
                    rex.w = true;
                    break;

                default:
                    throw Exception::InternalError("Invalid operand size", line, column, file.c_str());
            }

            if (instr == x86::Instruction::TEST)
            {
                if (immediate.sizeInBits == 8)
                    opcode = 0xF6;
                else
                    opcode = 0xF7;

                modrm.use = true;
                modrm.reg = 0;
            }
            else
            {
                if (immediate.sizeInBits == 8)
                    opcode = 0x80;
                else
                {
                    canOptimize = true;
                    opcode = 0x81;
                }

                modrm.use = true;

                switch (instr)
                {
                    case x86::Instruction::ADD: modrm.reg = 0; break;
                    case x86::Instruction::OR:  modrm.reg = 1; break;
                    case x86::Instruction::ADC: modrm.reg = 2; break;
                    case x86::Instruction::SBB: modrm.reg = 3; break;
                    case x86::Instruction::AND: modrm.reg = 4; break;
                    case x86::Instruction::SUB: modrm.reg = 5; break;
                    case x86::Instruction::XOR: modrm.reg = 6; break;
                    case x86::Instruction::CMP: modrm.reg = 7; break;
                    default:
                        throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
                }
            }
        }
    }
    else
    {
        uint64_t mainSize;
        uint64_t otherSize;

        if (mainOperand.isRegister())
            mainSize = parseRegister(mainOperand.getRegister(), true, useRMFirst);
        else // memory
            mainSize = parseMemory(mainOperand.getMemory());

        if (otherOperand.isRegister())
            otherSize = parseRegister(otherOperand.getRegister(), true, !useRMFirst);
        else // memory
            otherSize = parseMemory(otherOperand.getMemory(), mainSize);

        if (mainSize == AsmProg::Memory::POINTER_SIZE_NONE)
            mainSize = otherSize;

        if (mainSize != otherSize)
            throw Exception::SemanticError("Cannot use instruction with operands of different sizes", line, column, file.c_str());

        switch (mainSize)
        {
            case 8: break;

            case 16:
                if (bits != BitMode::Bits16) use16BitPrefix = true;
                break;

            case 32:
                if (bits == BitMode::Bits16) use16BitPrefix = true;
                break;

            case 64:
                rex.use = true;
                rex.w = true;
                break;

            default:
                throw Exception::InternalError("Invalid operand size", line, column, file.c_str());
        }

        if (mainSize == 8)
        {
            switch (instr)
            {
                case x86::Instruction::ADD:  opcode = (useRMFirst ? 0x00 : 0x02); break;
                case x86::Instruction::OR:   opcode = (useRMFirst ? 0x08 : 0x0A); break;
                case x86::Instruction::ADC:  opcode = (useRMFirst ? 0x10 : 0x12); break;
                case x86::Instruction::SBB:  opcode = (useRMFirst ? 0x18 : 0x1A); break;
                case x86::Instruction::AND:  opcode = (useRMFirst ? 0x20 : 0x22); break;
                case x86::Instruction::SUB:  opcode = (useRMFirst ? 0x28 : 0x2A); break;
                case x86::Instruction::XOR:  opcode = (useRMFirst ? 0x30 : 0x32); break;
                case x86::Instruction::CMP:  opcode = (useRMFirst ? 0x38 : 0x3A); break;
                case x86::Instruction::TEST: opcode = 0x84; break; // commutative
                default:
                    throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
            }
        }
        else
        {
            switch (instr)
            {
                case x86::Instruction::ADD:  opcode = (useRMFirst ? 0x01 : 0x03); break;
                case x86::Instruction::OR:   opcode = (useRMFirst ? 0x09 : 0x0B); break;
                case x86::Instruction::ADC:  opcode = (useRMFirst ? 0x11 : 0x13); break;
                case x86::Instruction::SBB:  opcode = (useRMFirst ? 0x19 : 0x1B); break;
                case x86::Instruction::AND:  opcode = (useRMFirst ? 0x21 : 0x23); break;
                case x86::Instruction::SUB:  opcode = (useRMFirst ? 0x29 : 0x2B); break;
                case x86::Instruction::XOR:  opcode = (useRMFirst ? 0x31 : 0x33); break;
                case x86::Instruction::CMP:  opcode = (useRMFirst ? 0x39 : 0x3B); break;
                case x86::Instruction::TEST: opcode = 0x85; break; // commutative
                default:
                    throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
            }
        }

        if (mainOperand.isRegister()) checkRegFor8BitREX(mainOperand.getRegister());
        if (otherOperand.isRegister()) checkRegFor8BitREX(otherOperand.getRegister());
    }
}

bool Assembler::x86::Two_Argument_ALU_Instruction::optimizeS()
{
    if (canOptimize && !immediate.needsRelocation)
    {
        int32_t value;
        if (immediate.sizeInBits == 16)
            value = static_cast<int32_t>(static_cast<int16_t>(static_cast<uint16_t>(immediate.value)));
        else
            value = static_cast<int32_t>(static_cast<uint32_t>(immediate.value));

        if (
            value <= static_cast<int32_t>(std::numeric_limits<int8_t>::max()) &&
            value >= static_cast<int32_t>(std::numeric_limits<int8_t>::min())
        ) {
            canOptimize = false;

            opcode = 0x83;

            immediate.sizeInBits = 8;

            if (accumulatorReg)
                (void)parseRegister(mainOp.getRegister(), true, true);

            return true;
        }
    }

    return false;
}
