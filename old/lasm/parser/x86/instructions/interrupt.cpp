#include "../parser.hpp"

#include "../../../instruction/x86/interrupt/interrupt.hpp"

Assembler::AsmProg::SectionEntry Assembler::x86::Parser::parseInterruptInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i)
{
    const uint64_t line = tokens[i].line;
    const uint64_t column = tokens[i].column;
    const StringPool::String file = tokens[i].file;
    i++;
    
    std::shared_ptr<Assembler::Instruction> instr;

    switch (instruction)
    {
        case Instruction::INT:
        {
            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            const uint64_t oldIndex = i;
            if (parseRegister(tokens, i).has_value())
                throw Exception::SyntaxError("Can't use registers for interrupt instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            else if (parseMemory(tokens, i).has_value())
                throw Exception::SyntaxError("Can't use memory operands for interrupt instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
                
            std::optional<AsmProg::Immediate> immediate = parseImmediate(tokens, i);
            if (!immediate.has_value())
                throw Exception::SyntaxError("Invalid immediate", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            instr = std::make_shared<Normal_Interrupt_Instruction>(instruction, currentBitMode, immediate.value(), line, column, file);
            break;
        }

        case Instruction::INT3: case Instruction::INTO:
        case Instruction::INT1: case Instruction::IRET:
        case Instruction::IRETQ: case Instruction::IRETD:
        case Instruction::SYSCALL: case Instruction::SYSRET:
        case Instruction::SYSENTER: case Instruction::SYSEXIT:
            instr = std::make_shared<Simple_Interrupt_Instruction>(instruction, currentBitMode, line, column, file);
            break;

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    if (!instr)
        throw Exception::InternalError("Instruction is nullptr", line, column, file.c_str());
    
    return instr;
}
