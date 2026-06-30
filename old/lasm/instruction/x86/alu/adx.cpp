#include "alu.hpp"

#include <Exception.hpp>

Assembler::x86::ADX_Alu_Instruction::ADX_Alu_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Register& regOperand, const AsmProg::Operand& otherOperand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::ADCX && instr != x86::Instruction::ADOX)
        throw Exception::InternalError("Invalid ADX ALU instruction", line, column, file.c_str());

    if (instr == x86::Instruction::ADCX)
        use66OpcodeOverride = true;
    else // ADOX
        useF3OpcodeOverride = true;

    opcodeEscape = OpcodeEscape::THREE_BYTE_38;
    opcode = 0xF6;

    uint64_t mainSize = parseRegister(regOperand, true, false);
    uint64_t otherSize;
    if (otherOperand.isRegister())
        otherSize = parseRegister(otherOperand.getRegister(), true, true);
    else if (otherOperand.isMemory())
        otherSize = parseMemory(otherOperand.getMemory(), mainSize);
    else
        throw Exception::InternalError("Wrong operand type for instruction", line, column, file.c_str());

    if (mainSize != otherSize)
        throw Exception::SemanticError("Cannot use instruction with operands of different sizes", line, column, file.c_str());

    checkSize(mainSize);

    if (mainSize == 64)
    {
        rex.use = true;
        rex.w = true;
    }
    else if (mainSize != 32)
        throw Exception::SemanticError("Instruction only allows 32 or 64 operands", line, column, file.c_str());
}
