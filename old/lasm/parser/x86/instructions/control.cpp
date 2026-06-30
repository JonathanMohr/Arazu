#include "../parser.hpp"

#include "../../../instruction/x86/control/control.hpp"

Assembler::AsmProg::SectionEntry Assembler::x86::Parser::parseControlInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i)
{
    const uint64_t line = tokens[i].line;
    const uint64_t column = tokens[i].column;
    const StringPool::String file = tokens[i].file;
    i++;
    
    std::shared_ptr<Assembler::Instruction> instr;

    switch (instruction)
    {
        case Instruction::NOP: case Instruction::HLT:
            instr = std::make_shared<Simple_Control_Instruction>(instruction, currentBitMode, line, column, file);
            break;

        case Instruction::CALL:
        {
            if (i < tokens.size() && tokens[i].type == Tokens::Type::Token)
            {
                if (tokens[i].value == "short")
                    throw Exception::SyntaxError("Short calls don't exist", line, column, file.c_str());
                else if (tokens[i].value == "near")
                { /* TODO: default */ }
                else if (tokens[i].value == "far")
                    throw Exception::SyntaxError("Far calls are not supported yet", line, column, file.c_str());
                else
                    i--;
                i++;
            }

            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            std::optional<AsmProg::Operand> operand = std::nullopt;;

            if (std::optional<AsmProg::Register> reg = parseRegister(tokens, i); reg.has_value())
                operand = reg;
            else if (std::optional<AsmProg::Memory> mem = parseMemory(tokens, i); mem.has_value())
                operand = mem;
            else if (std::optional<AsmProg::Immediate> imm = parseImmediate(tokens, i); imm.has_value())
                operand = imm;
            
            if (!operand.has_value())
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            instr = std::make_shared<CALL_Instruction>(instruction, currentBitMode, operand.value(), line, column, file);
            break;
        }

        case Instruction::RET:
        {
            if (i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
            {
                const uint64_t oldIndex = i;
                if (parseRegister(tokens, i).has_value())
                    throw Exception::SyntaxError("Can't use registers for RET instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
                else if (parseMemory(tokens, i).has_value())
                    throw Exception::SyntaxError("Can't use memory operands for RET instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
                
                std::optional<AsmProg::Immediate> immediate = parseImmediate(tokens, i);
                if (!immediate.has_value())
                    throw Exception::SyntaxError("Invalid immediate", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

                instr = std::make_shared<RET_Instruction>(instruction, currentBitMode, immediate.value(), line, column, file);
            }
            else
                instr = std::make_shared<RET_Instruction>(instruction, currentBitMode, line, column, file);

            break;
        }

        case Instruction::JMP:
        {
            bool forceLength = false;
            bool isNear = true;

            if (i < tokens.size() && tokens[i].type == Tokens::Type::Token)
            {
                if (tokens[i].value == "short")
                {
                    forceLength = true;
                    isNear = false;
                }
                else if (tokens[i].value == "near")
                    forceLength = true;
                else if (tokens[i].value == "far")
                    throw Exception::SyntaxError("Far jumps are not supported yet", line, column, file.c_str());
                else
                    i--;
                i++;
            }

            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            std::optional<AsmProg::Operand> operand = std::nullopt;;

            if (std::optional<AsmProg::Register> reg = parseRegister(tokens, i); reg.has_value())
                operand = reg;
            else if (std::optional<AsmProg::Memory> mem = parseMemory(tokens, i); mem.has_value())
                operand = mem;
            else if (std::optional<AsmProg::Immediate> imm = parseImmediate(tokens, i); imm.has_value())
                operand = imm;
            
            if (!operand.has_value())
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            if (forceLength && !isNear && !operand.value().isImmediate())
                throw Exception::SemanticError("Short jumps only exist for immediates", line, column, file.c_str());

            if (forceLength)
                instr = std::make_shared<JMP_Instruction>(instruction, isNear, currentBitMode, operand.value(), line, column, file);
            else
                instr = std::make_shared<JMP_Instruction>(instruction, currentBitMode, operand.value(), line, column, file);
            break;
        }

        case Instruction::JE: case Instruction::JNE:
        case Instruction::JG: case Instruction::JGE:
        case Instruction::JL: case Instruction::JLE:
        case Instruction::JA: case Instruction::JAE:
        case Instruction::JB: case Instruction::JBE:
        case Instruction::JO: case Instruction::JNO:
        case Instruction::JS: case Instruction::JNS:
        case Instruction::JP: case Instruction::JNP:
        case Instruction::JC: case Instruction::JNC:
        {
            bool forceLength = false;
            bool isNear = true;

            if (i < tokens.size() && tokens[i].type == Tokens::Type::Token)
            {
                if (tokens[i].value == "short")
                {
                    forceLength = true;
                    isNear = false;
                }
                else if (tokens[i].value == "near")
                    forceLength = true;
                else if (tokens[i].value == "far")
                    throw Exception::SyntaxError("Far jumps are not supported yet", line, column, file.c_str());
                else
                    i--;
                i++;
            }

            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            const uint64_t oldIndex = i;
            if (parseRegister(tokens, i).has_value())
                throw Exception::SyntaxError("Can't use registers for conditional JMP instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            else if (parseMemory(tokens, i).has_value())
                throw Exception::SyntaxError("Can't use memory operands for conditional JMP instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
                
            std::optional<AsmProg::Immediate> immediate = parseImmediate(tokens, i);
            if (!immediate.has_value())
                throw Exception::SyntaxError("Invalid immediate", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            if (forceLength)
                instr = std::make_shared<JMP_Instruction>(instruction, isNear, currentBitMode, immediate.value(), line, column, file);
            else
                instr = std::make_shared<JMP_Instruction>(instruction, currentBitMode, immediate.value(), line, column, file);
            break;
        }

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    if (!instr)
        throw Exception::InternalError("Instruction is nullptr", line, column, file.c_str());
    
    return instr;
}
