#include "alu.hpp"

#include <Exception.hpp>

Assembler::x86::Shift_Rotate_ALU_Instruction::Shift_Rotate_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (
        instr != x86::Instruction::SHL && instr != x86::Instruction::SHR &&
        instr != x86::Instruction::SAL && instr != x86::Instruction::SAR &&
        instr != x86::Instruction::ROL && instr != x86::Instruction::ROR &&
        instr != x86::Instruction::RCL && instr != x86::Instruction::RCR
    ) throw Exception::InternalError("Invalid Shift/Rotate ALU instruction", line, column, file.c_str());

    uint64_t opSize;
    if (operand.isRegister())
        opSize = parseRegister(operand.getRegister(), true, true);
    else if (operand.isMemory())
        opSize = parseMemory(operand.getMemory(), AsmProg::Memory::POINTER_SIZE_NONE);
    else
        throw Exception::SyntaxError("Cannot use immediate as operand for instruction", line, column, file.c_str());

    switch (opSize)
    {
        case 8:
            break;

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

    if (opSize == 8) is8Bit = true;

    if (opSize == 8) opcode = 0xC0;
    else             opcode = 0xC1;

    modrm.use = true;
    switch (instr)
    {
        case x86::Instruction::ROL: modrm.reg = 0; break;
        case x86::Instruction::ROR: modrm.reg = 1; break;
        case x86::Instruction::RCL: modrm.reg = 2; break;
        case x86::Instruction::RCR: modrm.reg = 3; break;
        case x86::Instruction::SHL: case x86::Instruction::SAL:
            modrm.reg = 4; break;
        case x86::Instruction::SHR: modrm.reg = 5; break;
        case x86::Instruction::SAR: modrm.reg = 7; break;
        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    immediate.use = true;
    immediate.sizeInBits = 8;
    immediate.immediate = imm;

    canOptimize = true;
}

Assembler::x86::Shift_Rotate_ALU_Instruction::Shift_Rotate_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (
        instr != x86::Instruction::SHL && instr != x86::Instruction::SHR &&
        instr != x86::Instruction::SAL && instr != x86::Instruction::SAR &&
        instr != x86::Instruction::ROL && instr != x86::Instruction::ROR &&
        instr != x86::Instruction::RCL && instr != x86::Instruction::RCR
    ) throw Exception::InternalError("Invalid Shift/Rotate ALU instruction", line, column, file.c_str());

    uint64_t opSize;
    if (operand.isRegister())
        opSize = parseRegister(operand.getRegister(), true, true);
    else if (operand.isMemory())
        opSize = parseMemory(operand.getMemory(), AsmProg::Memory::POINTER_SIZE_NONE);
    else
        throw Exception::SyntaxError("Cannot use immediate as operand for instruction", line, column, file.c_str());

    switch (opSize)
    {
        case 8:
            break;

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

    if (opSize == 8) is8Bit = true;

    if (opSize == 8) opcode = 0xD2;
    else             opcode = 0xD3;

    modrm.use = true;
    switch (instr)
    {
        case x86::Instruction::ROL: modrm.reg = 0; break;
        case x86::Instruction::ROR: modrm.reg = 1; break;
        case x86::Instruction::RCL: modrm.reg = 2; break;
        case x86::Instruction::RCR: modrm.reg = 3; break;
        case x86::Instruction::SHL: case x86::Instruction::SAL:
            modrm.reg = 4; break;
        case x86::Instruction::SHR: modrm.reg = 5; break;
        case x86::Instruction::SAR: modrm.reg = 7; break;
        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }
}

bool Assembler::x86::Shift_Rotate_ALU_Instruction::optimizeS()
{
    if (canOptimize && !immediate.needsRelocation)
    {
        if (static_cast<uint8_t>(immediate.value) == 1)
        {
            canOptimize = false;

            immediate.use = false;

            if (is8Bit) opcode = 0xD0;
            else        opcode = 0xD1;

            return true;
        }
    }

    return false;
}
