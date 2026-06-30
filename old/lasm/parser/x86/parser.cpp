#include "parser.hpp"

#include "expression.hpp"

std::optional<Assembler::AsmProg::Register> Assembler::x86::Parser::parseRegister(const Tokens& tokens, uint64_t& i)
{
    auto it = registerMap.find(tokens[i].value.str());
    if (it == registerMap.end())
        return std::nullopt;

    i++;
    
    const Register reg = it->second;
    const uint64_t regVal = static_cast<uint64_t>(reg);

    AsmProg::Register r = regVal;

    return r;
}

std::optional<Assembler::AsmProg::Memory> Assembler::x86::Parser::parseMemory(const Tokens& tokens, uint64_t& i)
{
    static const std::unordered_map<std::string_view, uint64_t> pointer_sizes = {
        {"byte", 8}, {"word", 16}, {"dword", 32}, {"qword", 64}
    };

    uint64_t pointerSize = AsmProg::Memory::POINTER_SIZE_NONE;
    auto ptrsizeIt = pointer_sizes.find(tokens[i].value.str());
    if (ptrsizeIt != pointer_sizes.end())
    {
        pointerSize = ptrsizeIt->second;
        i++;
        if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
            throw Exception::SyntaxError("Unexpected end of line after pointer size", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
    }

    if ((tokens[i].type == Tokens::Type::Bracket && tokens[i].value == "["))
    {
        AsmProg::Memory memory;

        i++;
        if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
            throw Exception::SyntaxError("Unexpected end of line after opening bracket", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

        memory.pointerSize = pointerSize;

        // TODO: Segment registers

        uint64_t startIndex = i;

        while (i < tokens.size() && !(tokens[i].type == Tokens::Type::Bracket && tokens[i].value == "]") && !(tokens[i].type == Tokens::Type::EOL))
        {
            if (
                tokens[i].type == Tokens::Type::Token || tokens[i].type == Tokens::Type::Operator ||
                (tokens[i].type == Tokens::Type::Bracket && (tokens[i].value == "(" || tokens[i].value == ")"))
            )
            {
                i++;
                continue;
            }

            throw Exception::SyntaxError("Invalid token in memory operand: " + tokens[i].value.str(), tokens[i].line, tokens[i].column, tokens[i].file.c_str());
        }

        if (tokens[i].type == Tokens::Type::EOL)
            throw Exception::SyntaxError("Missing closing bracket for memory operand", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

        ExpressionParser exprParser;
        std::shared_ptr<ExpressionParser::ExprNode> ast = exprParser.parse(tokens, startIndex, i, tokens[startIndex].line, tokens[startIndex].column, tokens[startIndex].file);

        if (!ast)
            throw Exception::SyntaxError("Empty memory operand", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

        ast = exprParser.simplify(ast);

        ExpressionParser::AddressingMode mode = exprParser.extractAddressingMode(ast);

        memory.used = AsmProg::Memory::Used::UNSURE;

        if (mode.has_reg1)
        {
            memory.useReg1 = true;
            memory.reg1 = mode.reg1;
            memory.scale1 = exprParser.convertToImmediate(mode.scale1, context);
        }

        if (mode.has_reg2)
        {
            memory.useReg2 = true;
            memory.reg2 = mode.reg2;
            memory.scale2 = exprParser.convertToImmediate(mode.scale2, context);
        }

        if (mode.has_displacement)
        {
            memory.useDisplacement = true;
            memory.displacement = exprParser.convertToImmediate(mode.displacement, context);
        }

        if (i < tokens.size()) i++; // ']'

        return memory;
    }

    if (ptrsizeIt != pointer_sizes.end())
        throw Exception::SyntaxError("Expected memory operand after pointer size", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

    return std::nullopt;
}

std::optional<Assembler::AsmProg::Immediate> Assembler::x86::Parser::parseImmediate(const Tokens& tokens, uint64_t& i)
{
    AsmProg::Immediate immediate;

    while (i < tokens.size() && tokens[i].type != Tokens::Type::EOL && tokens[i].type != Tokens::Type::Comma)
    {
        AsmProg::Immediate::Operand op = getImmediateOperand(tokens[i]);
        immediate.operands.push_back(op);
        i++;
    }

    if (immediate.operands.empty()) return std::nullopt;
    return immediate;
}
