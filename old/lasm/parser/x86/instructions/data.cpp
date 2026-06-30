#include "../parser.hpp"

#include "../../../instruction/x86/data/data.hpp"

Assembler::AsmProg::SectionEntry Assembler::x86::Parser::parseDataInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i)
{
    const uint64_t line = tokens[i].line;
    const uint64_t column = tokens[i].column;
    const StringPool::String file = tokens[i].file;
    i++;
    
    std::shared_ptr<Assembler::Instruction> instr;

    switch (instruction)
    {
        case Instruction::MOV:
        {
            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            std::optional<AsmProg::Operand> destinationOperand = std::nullopt;

            uint64_t oldIndex = i;

            if (std::optional<AsmProg::Register> reg = parseRegister(tokens, i); reg.has_value())
                destinationOperand = reg;
            else if (std::optional<AsmProg::Memory> mem = parseMemory(tokens, i); mem.has_value())
                destinationOperand = mem;
            else if (std::optional<AsmProg::Immediate> imm = parseImmediate(tokens, i); imm.has_value())
                throw Exception::SyntaxError("Cannot use immediate as destination operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            
            if (!destinationOperand.has_value())
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            if (i >= tokens.size() || tokens[i].type != Tokens::Type::Comma)
                throw Exception::SyntaxError("Expected ',' after first operand for instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
            i++;
            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Unexpected end of line after ','", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            oldIndex = i;

            std::optional<AsmProg::Operand> otherOperand = std::nullopt;

            if (std::optional<AsmProg::Register> reg = parseRegister(tokens, i); reg.has_value())
                otherOperand = reg;
            else if (std::optional<AsmProg::Memory> mem = parseMemory(tokens, i); mem.has_value())
                otherOperand = mem;
            else if (std::optional<AsmProg::Immediate> imm = parseImmediate(tokens, i); imm.has_value())
                otherOperand = imm;
            
            if (!otherOperand.has_value())
                throw Exception::SyntaxError("Expected second operand for instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            instr = std::make_shared<Mov_Instruction>(instruction, currentBitMode, destinationOperand.value(), otherOperand.value(), line, column, file);
            break;
        }

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    if (!instr)
        throw Exception::InternalError("Instruction is nullptr", line, column, file.c_str());
    
    return instr;
}
