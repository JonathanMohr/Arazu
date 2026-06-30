#include "../parser.hpp"

#include "../../../instruction/x86/flag/flag.hpp"

Assembler::AsmProg::SectionEntry Assembler::x86::Parser::parseFlagInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i)
{
    const uint64_t line = tokens[i].line;
    const uint64_t column = tokens[i].column;
    const StringPool::String file = tokens[i].file;
    i++;
    
    std::shared_ptr<Assembler::Instruction> instr;

    switch (instruction)
    {
        case Instruction::CLC: case Instruction::STC: case Instruction::CMC:
        case Instruction::CLD: case Instruction::STD:
        case Instruction::CLI: case Instruction::STI:
        case Instruction::LAHF: case Instruction::SAHF:
            instr = std::make_shared<Simple_Flag_Instruction>(instruction, currentBitMode, line, column, file);
            break;

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    if (!instr)
        throw Exception::InternalError("Instruction is nullptr", line, column, file.c_str());
    
    return instr;
}
