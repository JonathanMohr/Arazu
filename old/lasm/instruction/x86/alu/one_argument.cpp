#include "alu.hpp"

#include <Exception.hpp>

Assembler::x86::One_Argument_ALU_Instruction::One_Argument_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::NOT && instr != x86::Instruction::NEG && instr != x86::Instruction::INC && instr != x86::Instruction::DEC)
        throw Exception::InternalError("Invalid one argument ALU instruction", line, column, file.c_str());

    uint64_t size;
    if (operand.isRegister())
        size = parseRegister(operand.getRegister(), true, true);
    else if (operand.isMemory())
        size = parseMemory(operand.getMemory(), AsmProg::Memory::POINTER_SIZE_NONE);
    else
        throw Exception::SyntaxError("Cannot use instruction with immediates", line, column, file.c_str());

    if (size == 8)
    {
        if (instr == x86::Instruction::NOT || instr == x86::Instruction::NEG)
            opcode = 0xF6;
        else // INC, DEC
            opcode = 0xFE;
    }
    else
    {
        if (instr == x86::Instruction::NOT || instr == x86::Instruction::NEG)
            opcode = 0xF7;
        else // INC, DEC
            opcode = 0xFF;
    }

    modrm.use = true;
    switch (instr)
    {
        case x86::Instruction::NOT: modrm.reg = 2; break;
        case x86::Instruction::NEG: modrm.reg = 3; break;

        case x86::Instruction::INC: modrm.reg = 0; break;
        case x86::Instruction::DEC: modrm.reg = 1; break;

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    checkSize(size);

    switch (size)
    {
        case 8: break;
        
        case 16:
            if (bits != BitMode::Bits16)
                use16BitPrefix = true;
            break;

        case 32:
            if (bits == BitMode::Bits16)
                use16BitPrefix = true;
            break;

        case 64:
            rex.use = true;
            rex.w = true;
            break;

        default:
            throw Exception::InternalError("Invalid size", line, column, file.c_str());
    }

    if (operand.isRegister() && bits != BitMode::Bits64 && size != 8 &&
        (instr == x86::Instruction::INC || instr == x86::Instruction::DEC))
    {
        modrm.use = false;

        auto [regIndex, regUseREX, regSetREX] = getRegInfo(operand.getRegister(), line, column, file.c_str());
        if (instr == x86::Instruction::INC)
            opcode = 0x40 + regIndex;
        else // DEC
            opcode = 0x48 + regIndex;
    }
}
