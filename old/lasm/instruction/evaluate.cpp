#include "evaluate.hpp"

#include <Exception.hpp>
#include <stack>

Assembler::Evaluator::Result Assembler::Evaluator::Evaluate(const AsmProg::Immediate& immediate, Evaluator::Context& context, bool pcRelative, uint64_t pcExtra)
{
    // TODO: Think about a better way

    ShuntingYard::PreparedTokens tokens = ShuntingYard::prepareTokens(immediate, context);

    if (tokens.relocationPossible)
    {
        uint64_t off1 = 348234582348; // Random
        if (tokens.useSection && !tokens.isExtern)
        {
            auto it = context.sectionStarts.find(tokens.usedSection.str());
            if (it == context.sectionStarts.end()) throw Exception::InternalError("Couldn't find start of section '" + tokens.usedSection.str() + "'", context.line, context.column, context.file.c_str());
            off1 = it->second;
        }
        uint64_t off2 = off1 + 212483184284; // Random

        uint64_t res1 = ShuntingYard::evaluate(tokens, off1, context.line, context.column, context.file);
        uint64_t res2 = ShuntingYard::evaluate(tokens, off2, context.line, context.column, context.file);

        Result result;
        result.usedSection = tokens.usedSection;
        result.isExtern = tokens.isExtern;

        if (pcRelative)
        {
            uint64_t pcBase1 = context.bytesWritten + pcExtra;
            uint64_t pcBase2 = context.bytesWritten + pcExtra + (off2 - off1);

            res1 = res1 - pcBase1;
            res2 = res2 - pcBase2;

            result.result = res1;

            if (res1 == res2 && tokens.useSection && context.currentSection == result.usedSection)
            {
                result.useOffset = false;
                result.relocationPossible = true;
                result.offset = 0;
            }
            else if (!tokens.useSection)
            {
                result.useOffset = false;
                result.relocationPossible = true;

                result.result = res1 + pcBase1 - context.sectionOffset - pcExtra;
            }
            else if (!tokens.isExtern)
            {
                result.useOffset = true;
                result.relocationPossible = true;
                result.offset = static_cast<int64_t>(res1 - off1 + pcExtra);
            }
            else // TODO: Check
            {
                result.useOffset = true;
                result.relocationPossible = true;
                result.offset = 0;
            }
        }
        else
        {
            result.result = res1;

            if (res1 == res2)
            {
                result.useOffset = false;
                result.relocationPossible = true;
                result.offset = 0;
            }
            else if ((res1 - off1) == (res2 - off2))
            {
                result.useOffset = true;
                result.relocationPossible = true;
                result.offset = static_cast<int64_t>(res1 - off1);
            }
            else
            {
                result.useOffset = false;
                result.relocationPossible = false;
                result.offset = 0;
            }
        }

        if (result.relocationPossible && result.isExtern && tokens.useSection)
        {
            auto it = context.labelMap.find(result.usedSection.str());
            if (it != context.labelMap.end())
            {
                AsmProg::Label& label = context.symbols[it->second].getLabel();
                label.externUsed = true;
            }
        }

        return result;
    }
    else
    {
        uint64_t res = ShuntingYard::evaluate(tokens, context.bytesWritten - context.sectionOffset, context.line, context.column, context.file);

        Result result;
        result.result = res;

        result.offset = 0;
        result.usedSection = tokens.usedSection;
        result.useOffset = false;
        result.relocationPossible = false;
        result.isExtern = false;

        return result;
    }
}

static int precedence(char op)
{
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '^') return 3;
    return 0;
}

static bool isLeftAssociative(char op)
{
    return op != '^';
}

Assembler::ShuntingYard::PreparedTokens Assembler::ShuntingYard::prepareTokens(const AsmProg::Immediate& immediate, Evaluator::Context& context)
{
    PreparedTokens output;
    output.relocationPossible = true;

    std::vector<Token> outputQueue;
    std::stack<char> operatorStack;

    StringPool::String usedSection;
    bool useSection = false;

    bool expectUnaryMinus = false;

    for (uint64_t i = 0; i < immediate.operands.size(); i++)
    {
        const AsmProg::Immediate::Operand& op = immediate.operands[i];

        if (op.isOperator())
        {
            const char opChar = op.getOperator().op;

            if (opChar == '-' && (
                i == 0 ||
                (immediate.operands[i - 1].isOperator() && immediate.operands[i - 1].getOperator().op != ')')
            )) expectUnaryMinus = true;
            else if (opChar == '(') operatorStack.push(opChar);
            else if (opChar == ')')
            {
                while (!operatorStack.empty() && operatorStack.top() != '(')
                {
                    outputQueue.emplace_back(operatorStack.top());
                    operatorStack.pop();
                }
                if (operatorStack.empty())
                    throw Exception::SyntaxError("Mismatched parentheses", context.line, context.column, context.file.c_str());
                operatorStack.pop();
            }
            else
            {
                // other operator
                while (!operatorStack.empty())
                {
                    char topOp = operatorStack.top();
                    if (topOp == '(')
                        break;

                    int topPrec = precedence(topOp);
                    int currPrec = precedence(opChar);

                    if ((isLeftAssociative(opChar) && currPrec <= topPrec) ||
                        (!isLeftAssociative(opChar) && currPrec < topPrec))
                    {
                        outputQueue.emplace_back(topOp);
                        operatorStack.pop();
                    }
                    else break;
                }
                operatorStack.push(opChar);
            }
        }
        else if (op.isInteger())
        {
            uint64_t val = op.getInteger().value;
            if (expectUnaryMinus)
            {
                // TODO: I don't know if it actually works
                val = ~val + 1; // Two's complement operation
                expectUnaryMinus = false;
            }
            outputQueue.emplace_back(val);
        }
        else if (op.isString())
        {
            StringPool::String name = op.getString().value;
            
            auto labelMapIt = context.labelMap.find(name.str());
            if (labelMapIt != context.labelMap.end())
            {
                AsmProg::Label& label = context.symbols[labelMapIt->second].getLabel();
                if (label.isExtern)
                {
                    if (useSection && label.name != usedSection)
                        output.relocationPossible = false;
                    
                    Token token;
                    token.type = Token::Type::Position;
                    token.offset = 0;
                    if (expectUnaryMinus)
                    {
                        token.negative = true;
                        expectUnaryMinus = false;
                    }

                    outputQueue.push_back(token);
                    usedSection = label.name;
                    useSection = true;
                    output.isExtern = true;
                }
                else
                {
                    if (useSection && label.section != usedSection)
                        output.relocationPossible = false;

                    if (!label.resolved)
                        throw Exception::InternalError("Unresolved label '" + name.str() + "' used in expression", context.line, context.column, context.file.c_str());
                    
                    Token token;
                    token.type = Token::Type::Position;
                    token.offset = label.offset;
                    if (expectUnaryMinus)
                    {
                        token.negative = true;
                        expectUnaryMinus = false;
                    }
                    outputQueue.push_back(token);
                    usedSection = label.section;
                    useSection = true;
                    output.isExtern = false;
                }
            }
            else
            {
                auto constantIt = context.constantMap.find(name.str());
                if (constantIt != context.constantMap.end())
                {
                    const AsmProg::Constant& constant = context.symbols[constantIt->second].getConstant();

                    if ((useSection && constant.usedSection != usedSection && constant.hasPos == AsmProg::Constant::HasPos::TRUE) || (!constant.relocationPossible))
                        output.relocationPossible = false;

                    if (!constant.resolved)
                        throw Exception::InternalError("Unresolved constant '" + name.str() + "' used in expression", context.line, context.column, context.file.c_str());

                    if (constant.useOffset)
                    {
                        Token token;
                        token.type = Token::Type::Position;
                        token.offset = static_cast<uint64_t>(constant.off);
                        if (expectUnaryMinus)
                        {
                            token.negative = true;
                            expectUnaryMinus = false;
                        }

                        outputQueue.push_back(token);
                        usedSection = constant.usedSection;
                        useSection = true;
                    }
                    else
                    {
                        uint64_t val = static_cast<uint64_t>(constant.value);
                        if (expectUnaryMinus)
                        {
                            // TODO: I don't know if it actually works
                            val = ~val + 1; // Two's complement operation
                            expectUnaryMinus = false;
                        }
                        outputQueue.emplace_back(val);
                    }
                }
                else throw Exception::SemanticError("Unknown string '" + name.str() + "'", context.line, context.column, context.file.c_str());
            }
        }
        else if (op.isPosition())
        {
            const AsmProg::Position& pos = op.getPosition();
            Token token;
            token.type = Token::Type::Position;
            token.offset = pos.sectionPos ? 0 : context.sectionOffset;
            if (expectUnaryMinus)
            {
                token.negative = true;
                expectUnaryMinus = false;
            }
            outputQueue.push_back(token);

            if (useSection && context.currentSection != usedSection)
            {
                output.relocationPossible = false;
            }
            usedSection = context.currentSection;
            useSection = true;
        }
        else
        {
            throw Exception::InternalError("Expected value operand", context.line, context.column, context.file.c_str());
        }
    }

    // put rest in operatorStack to outputQueue
    while (!operatorStack.empty())
    {
        if (operatorStack.top() == '(' || operatorStack.top() == ')')
            throw Exception::SyntaxError("Mismatched parentheses", context.line, context.column, context.file.c_str());
        outputQueue.emplace_back(operatorStack.top());
        operatorStack.pop();
    }

    output.tokens = outputQueue;
    output.useSection = useSection;
    if (useSection) output.usedSection = usedSection;
    else            output.usedSection = context.currentSection;

    return output;
}

uint64_t Assembler::ShuntingYard::evaluate(const PreparedTokens& preparedTokens, uint64_t offset, uint64_t line, uint64_t column, const StringPool::String& file)
{
    std::stack<uint64_t> stack;

    for (const Token& token : preparedTokens.tokens)
    {
        if (token.type == Token::Type::Number)
            stack.push(token.number);
        else if (token.type == Token::Type::Position)
            stack.push((token.negative ? -token.offset : token.offset) + offset);
        else if (token.type == Token::Type::Operator)
        {
            if (stack.size() < 2)
                throw Exception::InternalError("Invalid expression: not enough operands", line, column, file.c_str());

            uint64_t rhs = stack.top(); stack.pop();
            uint64_t lhs = stack.top(); stack.pop();

            if (token.op == '+') stack.push(lhs + rhs);
            else if (token.op == '-') stack.push(lhs - rhs);
            else if (token.op == '*') stack.push(lhs * rhs);
            else if (token.op == '/') {
                if (rhs == 0)
                    throw Exception::SemanticError("Division by zero", line, column, file.c_str());
                stack.push(lhs / rhs);
            }
            else if (token.op == '%') {
                if (rhs == 0)
                    throw Exception::SemanticError("Modulo by zero", line, column, file.c_str());
                stack.push(lhs % rhs);
            }
            else
                throw Exception::InternalError(std::string("Invalid operator: ") + token.op, line, column, file.c_str());
        }
        else
            throw Exception::InternalError("Invalid token type", line, column, file.c_str());
    }

    if (stack.size() != 1) throw Exception::InternalError("Invalid expression", line, column, file.c_str());

    return stack.top();
}
