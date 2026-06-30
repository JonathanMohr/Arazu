#include "../parser.hpp"

#include "../../../instruction/x86/stack/stack.hpp"

Assembler::AsmProg::SectionEntry Assembler::x86::Parser::parseStackInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i)
{
    const uint64_t line = tokens[i].line;
    const uint64_t column = tokens[i].column;
    const StringPool::String file = tokens[i].file;
    i++;
    
    std::shared_ptr<Assembler::Instruction> instr;

    switch (instruction)
    {
        case Instruction::PUSH: case Instruction::POP:
        {
            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            std::optional<AsmProg::Operand> operand = std::nullopt;

            const uint64_t oldIndex = i;

            if (std::optional<AsmProg::Register> reg = parseRegister(tokens, i); reg.has_value())
                operand = reg;
            else if (std::optional<AsmProg::Memory> mem = parseMemory(tokens, i); mem.has_value())
                operand = mem;
            else if (std::optional<AsmProg::Immediate> imm = parseImmediate(tokens, i); imm.has_value())
                operand = imm;
            
            if (!operand.has_value())
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());
            
            if (instruction == Instruction::POP && operand.value().isImmediate())
                throw Exception::SyntaxError("Cannot pop into immediates", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            
            instr = std::make_shared<Normal_Stack_Instruction>(instruction, currentBitMode, operand.value(), line, column, file);
            break;
        }

        case Instruction::PUSHA: case Instruction::POPA:
        case Instruction::PUSHAD: case Instruction::POPAD:
        case Instruction::PUSHF: case Instruction::POPF:
        case Instruction::PUSHFD: case Instruction::POPFD:
        case Instruction::PUSHFQ: case Instruction::POPFQ:
            instr = std::make_shared<Simple_Stack_Instruction>(instruction, currentBitMode, line, column, file);
            break;

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    if (!instr)
        throw Exception::InternalError("Instruction is nullptr", line, column, file.c_str());
    
    return instr;
}
