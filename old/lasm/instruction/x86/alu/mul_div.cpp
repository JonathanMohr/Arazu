#include "alu.hpp"

#include <Exception.hpp>
#include <limits>

Assembler::x86::Mul_Div_ALU_Instruction::Mul_Div_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Register& reg, const AsmProg::Operand& operand, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::IMUL)
        throw Exception::InternalError("Invalid MUL/DIV ALU instruction with 3 operands", line, column, file.c_str());

    if (operand.isImmediate())
        throw Exception::SyntaxError("Cannot use immediates as second operand for imul", line, column, file.c_str());

    uint64_t mainSize = parseRegister(reg, true, false);
    uint64_t otherSize;
    if (operand.isRegister())
        otherSize = parseRegister(operand.getRegister(), true, true);
    else
        otherSize = parseMemory(operand.getMemory(), mainSize);

    if (mainSize != otherSize)
        throw Exception::SemanticError("Cannot use instruction with operands of different sizes", line, column, file.c_str());

    immediate.use = true;
    immediate.immediate = imm;

    opcode = 0x69;

    canOptimize = true;

    switch (mainSize)
    {
        case 8:
            throw Exception::SemanticError("Cannot use imul with 3 operands with 8-bit operands", line, column, file.c_str());

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
}

Assembler::x86::Mul_Div_ALU_Instruction::Mul_Div_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Register& reg, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::IMUL)
        throw Exception::InternalError("Invalid MUL/DIV ALU instruction with 2 operands", line, column, file.c_str());

    if (operand.isImmediate())
        throw Exception::SyntaxError("Cannot use immediates as second operand for imul", line, column, file.c_str());

    uint64_t mainSize = parseRegister(reg, true, false);
    uint64_t otherSize;
    if (operand.isRegister())
        otherSize = parseRegister(operand.getRegister(), true, true);
    else
        otherSize = parseMemory(operand.getMemory(), mainSize);

    if (mainSize != otherSize)
        throw Exception::SemanticError("Cannot use instruction with operands of different sizes", line, column, file.c_str());

    opcodeEscape = OpcodeEscape::TWO_BYTE;
    opcode = 0xAF;

    switch (mainSize)
    {
        case 8:
            throw Exception::SemanticError("Cannot use imul with 2 operands with 8-bit operands", line, column, file.c_str());

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
}

Assembler::x86::Mul_Div_ALU_Instruction::Mul_Div_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::MUL && instr != x86::Instruction::IMUL && instr != x86::Instruction::DIV && instr != x86::Instruction::IDIV)
        throw Exception::InternalError("Invalid MUL/DIV ALU instruction", line, column, file.c_str());

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

    if (opSize == 8) opcode = 0xF6;
    else             opcode = 0xF7;

    modrm.use = true;
    switch (instr)
    {
        case x86::Instruction::MUL:  modrm.reg = 4; break;
        case x86::Instruction::IMUL: modrm.reg = 5; break;
        case x86::Instruction::DIV:  modrm.reg = 6; break;
        case x86::Instruction::IDIV: modrm.reg = 7; break;

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }
}

bool Assembler::x86::Mul_Div_ALU_Instruction::optimizeS()
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

            opcode = 0x6B;

            immediate.sizeInBits = 8;

            return true;
        }
    }

    return false;
}
