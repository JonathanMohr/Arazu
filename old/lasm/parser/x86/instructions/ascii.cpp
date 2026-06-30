#include "../parser.hpp"

#include "../../../instruction/x86/ascii/ascii.hpp"

Assembler::AsmProg::SectionEntry Assembler::x86::Parser::parseASCIIInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i)
{
    const uint64_t line = tokens[i].line;
    const uint64_t column = tokens[i].column;
    const StringPool::String file = tokens[i].file;
    i++;
    
    std::shared_ptr<Assembler::Instruction> instr;

    switch (instruction)
    {
        case Instruction::AAA: case Instruction::AAS:
            instr = std::make_shared<ASCII_Instruction>(instruction, currentBitMode, line, column, file);
            break;

        case Instruction::AAD: case Instruction::AAM:
        {
            std::optional<AsmProg::Immediate> immediate;

            if (i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
            {
                const uint64_t oldIndex = i;
                if (parseRegister(tokens, i).has_value())
                    throw Exception::SyntaxError("Can't use registers for ASCII instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
                else if (parseMemory(tokens, i).has_value())
                    throw Exception::SyntaxError("Can't use memory operands for ASCII instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
                immediate = parseImmediate(tokens, i);
            }
            else
            {
                AsmProg::Immediate imm;
                AsmProg::Integer integer;
                integer.value = 10;
                imm.operands.push_back(integer);
                immediate = std::move(imm);
            }

            if (!immediate.has_value())
                throw Exception::SyntaxError("Invalid immediate", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            instr = std::make_shared<ASCII_Instruction>(instruction, currentBitMode, immediate.value(), line, column, file);
            break;
        }

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    if (!instr)
        throw Exception::InternalError("Instruction is nullptr", line, column, file.c_str());
    
    return instr;
}
