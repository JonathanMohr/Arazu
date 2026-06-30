#include "../parser.hpp"

#include "../../../instruction/x86/alu/alu.hpp"

Assembler::AsmProg::SectionEntry Assembler::x86::Parser::parseALUInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i)
{
    const uint64_t line = tokens[i].line;
    const uint64_t column = tokens[i].column;
    const StringPool::String file = tokens[i].file;
    i++;
    
    std::shared_ptr<Assembler::Instruction> instr;

    switch (instruction)
    {
        case Instruction::ADD: case Instruction::ADC:
        case Instruction::SUB: case Instruction::SBB:
        case Instruction::CMP: case Instruction::TEST:
        case Instruction::AND: case Instruction::OR:
        case Instruction::XOR:
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

            instr = std::make_shared<Two_Argument_ALU_Instruction>(instruction, currentBitMode, destinationOperand.value(), otherOperand.value(), line, column, file);
            break;
        }

        case Instruction::ADCX: case Instruction::ADOX:
        {
            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected operand for instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            std::optional<AsmProg::Register> destinationOperand = std::nullopt;

            uint64_t oldIndex = i;

            if (std::optional<AsmProg::Register> reg = parseRegister(tokens, i); reg.has_value())
                destinationOperand = reg;
            else if (std::optional<AsmProg::Memory> mem = parseMemory(tokens, i); mem.has_value())
                throw Exception::SyntaxError("Cannot use a memory operand as destination operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            else if (std::optional<AsmProg::Immediate> imm = parseImmediate(tokens, i); imm.has_value())
                throw Exception::SyntaxError("Cannot use an immediate as operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            
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
                throw Exception::SyntaxError("Cannot use an immediate as operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            
            if (!otherOperand.has_value())
                throw Exception::SyntaxError("Expected second operand for instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            instr = std::make_shared<ADX_Alu_Instruction>(instruction, currentBitMode, destinationOperand.value(), otherOperand.value(), line, column, file);
            break;
        }

        case Instruction::IMUL:
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
            {
                instr = std::make_shared<Mul_Div_ALU_Instruction>(instruction, currentBitMode, destinationOperand.value(), line, column, file);
                break;
            }
            else if (destinationOperand.value().isMemory())
                throw Exception::SyntaxError("Cannot use a memory operand as destination operand for instruction when using multiple operands", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());

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
                throw Exception::SyntaxError("Cannot use an immediate as operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            
            if (!otherOperand.has_value())
                throw Exception::SyntaxError("Expected second operand for instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            if (i >= tokens.size() || tokens[i].type != Tokens::Type::Comma)
            {
                instr = std::make_shared<Mul_Div_ALU_Instruction>(instruction, currentBitMode, destinationOperand.value().getRegister(), otherOperand.value(), line, column, file);
                break;
            }

            i++;
            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Unexpected end of line after ','", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            oldIndex = i;

            std::optional<AsmProg::Immediate> immediate = std::nullopt;

            if (std::optional<AsmProg::Register> reg = parseRegister(tokens, i); reg.has_value())
                throw Exception::SyntaxError("Cannot use a register as third operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            else if (std::optional<AsmProg::Memory> mem = parseMemory(tokens, i); mem.has_value())
                throw Exception::SyntaxError("Cannot use a memory operand as third operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            else if (std::optional<AsmProg::Immediate> imm = parseImmediate(tokens, i); imm.has_value())
                immediate = imm;
            
            if (!immediate.has_value())
                throw Exception::SyntaxError("Expected second operand for instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            instr = std::make_shared<Mul_Div_ALU_Instruction>(instruction, currentBitMode, destinationOperand.value().getRegister(), otherOperand.value(), immediate.value(), line, column, file);
            break;
        }

        case Instruction::MUL: case Instruction::DIV:
        case Instruction::IDIV:
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

            instr = std::make_shared<Mul_Div_ALU_Instruction>(instruction, currentBitMode, destinationOperand.value(), line, column, file);
            break;
        }

        case Instruction::SHL: case Instruction::SHR:
        case Instruction::SAL: case Instruction::SAR:
        case Instruction::ROL: case Instruction::ROR:
        case Instruction::RCL: case Instruction::RCR:
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
                throw Exception::SyntaxError("Cannot use immediate as operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            
            if (!destinationOperand.has_value())
                throw Exception::SyntaxError("Expected operand for instruction", line, column, file.c_str());

            if (i >= tokens.size() || tokens[i].type != Tokens::Type::Comma)
                throw Exception::SyntaxError("Expected ',' after first operand for instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            i++;
            if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                throw Exception::SyntaxError("Unexpected end of line after ','", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            oldIndex = i;

            if (std::optional<AsmProg::Register> reg = parseRegister(tokens, i); reg.has_value())
            {
                if (reg != Register::CL)
                    throw Exception::SyntaxError("Only CL is allowed as count operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
                instr = std::make_shared<Shift_Rotate_ALU_Instruction>(instruction, currentBitMode, destinationOperand.value(), line, column, file);
                break;
            }
            else if (std::optional<AsmProg::Memory> mem = parseMemory(tokens, i); mem.has_value())
                throw Exception::SyntaxError("Cannot use a memory operand as count for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());
            else if (std::optional<AsmProg::Immediate> imm = parseImmediate(tokens, i); imm.has_value())
            {
                instr = std::make_shared<Shift_Rotate_ALU_Instruction>(instruction, currentBitMode, destinationOperand.value(), imm.value(), line, column, file);
                break;
            }
            
            throw Exception::SyntaxError("Expected second operand for instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
        }

        case Instruction::NOT: case Instruction::NEG:
        case Instruction::INC: case Instruction::DEC:
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
                throw Exception::SyntaxError("Cannot use immediate as operand for instruction", tokens[oldIndex].line, tokens[oldIndex].column, tokens[oldIndex].file.c_str());

            instr = std::make_shared<One_Argument_ALU_Instruction>(instruction, currentBitMode, destinationOperand.value(), line, column, file);
            break;
        }

        default:
            throw Exception::InternalError("Invalid instruction", line, column, file.c_str());
    }

    if (!instr)
        throw Exception::InternalError("Instruction is nullptr", line, column, file.c_str());
    
    return instr;
}
