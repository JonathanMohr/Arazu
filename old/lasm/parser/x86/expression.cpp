#include "expression.hpp"

#include "../evaluate.hpp"
#include <unordered_map>
#include "../../instruction/x86.hpp"

const Assembler::Tokens::Token* Assembler::x86::ExpressionParser::peek() const
{
    if (current >= endIndex) return nullptr;
    return &(*tokens)[current];
}

const Assembler::Tokens::Token* Assembler::x86::ExpressionParser::advance()
{
    return &(*tokens)[current++];
}

std::shared_ptr<Assembler::x86::ExpressionParser::ExprNode> Assembler::x86::ExpressionParser::parse(const Tokens& tok, uint64_t startIndex, uint64_t e, uint64_t l, uint64_t c, StringPool::String f)
{
    line = l;
    column = c;
    file = f;

    tokens = &tok;
    current = startIndex;
    endIndex = e;
    return parseExpression();
}

void Assembler::x86::ExpressionParser::printTree(const std::shared_ptr<ExprNode>& e, int indent)
{
    if (!e) return;

    std::string indentStr(static_cast<size_t>(indent * 2), ' ');

    switch (e->type)
    {
        case ExprNode::Type::NUMBER:
            std::cerr << indentStr << "NUMBER: " << e->value << '\n';
            break;
            
        case ExprNode::Type::REGISTER:
            std::cerr << indentStr << "REGISTER: " << e->reg << '\n';
            break;
            
        case ExprNode::Type::LABEL:
            std::cerr << indentStr << "LABEL: " << e->name << '\n';
            break;
            
        case ExprNode::Type::BINARY_OP:
            std::cerr << indentStr << "BINARY_OP: '" << e->op << "'" << '\n';
            std::cerr << indentStr << "  left:" << '\n';
            printTree(e->left, indent + 2);
            std::cerr << indentStr << "  right:" << '\n';
            printTree(e->right, indent + 2);
            break;
            
        case ExprNode::Type::UNARY_OP:
            std::cerr << indentStr << "UNARY_OP: '" << e->unary_op << "'" << '\n';
            std::cerr << indentStr << "  operand:" << '\n';
            printTree(e->operand, indent + 2);
            break;
    }
}

std::shared_ptr<Assembler::x86::ExpressionParser::ExprNode> Assembler::x86::ExpressionParser::parseExpression()
{
    std::shared_ptr<ExprNode> left = parseTerm();

    while (peek() && peek()->type == Tokens::Type::Operator &&
           (peek()->value == "+" || peek()->value == "-"))
    {
        char op = advance()->value[0];
        std::shared_ptr<ExprNode> right = parseTerm();

        std::shared_ptr<ExpressionParser::ExprNode> node = std::make_shared<ExprNode>();
        node->type = ExprNode::Type::BINARY_OP;
        node->op = op;
        node->left = left;
        node->right = right;

        left = node;
    }

    return left;
}

std::shared_ptr<Assembler::x86::ExpressionParser::ExprNode> Assembler::x86::ExpressionParser::parseTerm()
{
    std::shared_ptr<ExprNode> left = parseUnary();

    while (peek() && peek()->type == Tokens::Type::Operator &&
           (peek()->value == "*" || peek()->value == "/" || peek()->value == "%"))
    {
        char op = advance()->value[0];
        std::shared_ptr<ExprNode> right = parseUnary();

        std::shared_ptr<ExpressionParser::ExprNode> node = std::make_shared<ExprNode>();
        node->type = ExprNode::Type::BINARY_OP;
        node->op = op;
        node->left = left;
        node->right = right;

        left = node;
    }

    return left;
}

std::shared_ptr<Assembler::x86::ExpressionParser::ExprNode> Assembler::x86::ExpressionParser::parseUnary()
{
    if (peek() && peek()->type == Tokens::Type::Operator &&
        (peek()->value == "+" || peek()->value == "-"))
    {
        char op = advance()->value[0];
        std::shared_ptr<ExprNode> operand = parseUnary();

        std::shared_ptr<ExpressionParser::ExprNode> node = std::make_shared<ExprNode>();
        node->type = ExprNode::Type::UNARY_OP;
        node->unary_op = op;
        node->operand = operand;

        return node;
    }

    return parsePrimary();
}

std::shared_ptr<Assembler::x86::ExpressionParser::ExprNode> Assembler::x86::ExpressionParser::parsePrimary()
{
    if (!peek())
        throw Exception::SyntaxError("Unexpected end of expression", line, column, file.c_str());

    if (peek() && peek()->type == Tokens::Type::Bracket && peek()->value == "(")
    {
        advance();
        std::shared_ptr<ExprNode> expr = parseExpression();
        if (!peek() || peek()->type != Tokens::Type::Bracket || peek()->value != ")")
            throw Exception::SyntaxError("Expected ')'", line, column, file.c_str());
        advance();
        return expr;
    }

    if (peek() && peek()->type == Tokens::Type::Token)
    {
        StringPool::String val = advance()->value;

        if (isNumber(val.c_str()))
        {
            std::shared_ptr<ExpressionParser::ExprNode> node = std::make_shared<ExprNode>();
            node->type = ExprNode::Type::NUMBER;
            node->value = evalInteger(val, 8, line, column, file.c_str());
            return node;
        }
        else if (isRegister(val.c_str()))
        {
            std::shared_ptr<ExpressionParser::ExprNode> node = std::make_shared<ExprNode>();
            node->type = ExprNode::Type::REGISTER;
            node->reg = getRegister(val.c_str());
            return node;
        }
        else
        {
            std::shared_ptr<ExpressionParser::ExprNode> node = std::make_shared<ExprNode>();
            node->type = ExprNode::Type::LABEL;
            node->name = val.c_str();
            return node;
        }
    }

    throw Exception::SyntaxError("Expected number, register or immediate", line, column, file.c_str());
}

bool Assembler::x86::ExpressionParser::isNumber(const std::string& s)
{
    return std::isdigit(static_cast<unsigned char>(s[0])) != 0;
}

bool Assembler::x86::ExpressionParser::isRegister(const std::string& s)
{
    auto regIt = registerMap.find(s);
    if (regIt != registerMap.end())
        return true;
    else
        return false;
}

uint64_t Assembler::x86::ExpressionParser::getRegister(const std::string& s)
{
    if (!isRegister(s)) throw Exception::InternalError("s is not a register", line, column, file.c_str());

    auto regIt = registerMap.find(s);
    if (regIt != registerMap.end())
        return static_cast<uint64_t>(regIt->second);
    
    throw Exception::InternalError("Invalid register", line, column, file.c_str());
}

// TODO: Add
std::shared_ptr<Assembler::x86::ExpressionParser::ExprNode> Assembler::x86::ExpressionParser::simplify(std::shared_ptr<ExprNode> e)
{
    if (!e) return nullptr;

    return e;
}

bool Assembler::x86::ExpressionParser::hasRegister(const std::shared_ptr<ExprNode>& e)
{
    if (!e) return false;
    if (e->type == ExprNode::Type::REGISTER) return true;

    return hasRegister(e->left) || hasRegister(e->right) || hasRegister(e->operand);
}

Assembler::AsmProg::Immediate Assembler::x86::ExpressionParser::convertToImmediate(const std::shared_ptr<ExprNode>& e, const Context& context)
{
    AsmProg::Immediate imm;

    if (!e)
        return imm;

    if (e->type == ExprNode::Type::REGISTER)
        throw Exception::InternalError("Can't convert register to immediate", line, column, file.c_str());

    if (e->type == ExprNode::Type::NUMBER)
    {
        AsmProg::Integer integer;
        integer.value = e->value;
        imm.operands.push_back(integer);
        return imm;
    }

    if (e->type == ExprNode::Type::LABEL)
    {
        if (e->name == "$" || e->name == "$$")
        {
            AsmProg::Position curPos;
            curPos.sectionPos = (e->name == "$") ? false : true;
            imm.operands.push_back(curPos);
        }
        else
        {
            AsmProg::String str;
            str.value = context.stringPool->GetString(e->name);
            imm.operands.push_back(str);
        }
        
        return imm;
    }

    if (e->type == ExprNode::Type::UNARY_OP)
    {
        AsmProg::Immediate operand = convertToImmediate(e->operand, context);

        AsmProg::Operator op;
        op.op = e->unary_op;
        
        AsmProg::Operator bOpen;
        bOpen.op = '(';
        AsmProg::Operator bClose;
        bClose.op = ')';

        imm.operands.push_back(bOpen);
        imm.operands.push_back(op);
        imm.operands.insert(imm.operands.end(), operand.operands.begin(), operand.operands.end());
        imm.operands.push_back(bClose);

        return imm;
    }

    if (e->type == ExprNode::Type::BINARY_OP)
    {
        AsmProg::Immediate left = convertToImmediate(e->left, context);
        AsmProg::Immediate right = convertToImmediate(e->right, context);

        AsmProg::Operator bOpen;
        bOpen.op = '(';
        AsmProg::Operator bClose;
        bClose.op =')';

        AsmProg::Operator op;
        op.op = e->op;

        imm.operands.push_back(bOpen);
        imm.operands.insert(imm.operands.end(), left.operands.begin(), left.operands.end());
        imm.operands.push_back(op);
        imm.operands.insert(imm.operands.end(), right.operands.begin(), right.operands.end());
        imm.operands.push_back(bClose);

        return imm;
    }

    throw Exception::InternalError("Unsupported node type in addressing mode", line, column, file.c_str());
}

namespace
{
    struct WorkItem
    {
        std::shared_ptr<Assembler::x86::ExpressionParser::ExprNode> node;
        int sign;
    };
}

Assembler::x86::ExpressionParser::AddressingMode Assembler::x86::ExpressionParser::extractAddressingMode(std::shared_ptr<ExprNode> e)
{
    AddressingMode mode;
    
    if (!e) return mode;

    std::unordered_map<uint64_t, std::vector<std::shared_ptr<ExprNode>>> regTerms;
    std::vector<uint64_t> regs;

    std::vector<std::shared_ptr<ExprNode>> constTerms;

    std::vector<WorkItem> stack = {{e, +1}};
    
    while (!stack.empty())
    {
        auto [node, sign] = stack.back();
        stack.pop_back();

        if (!node) continue;

        if (node->type == ExprNode::Type::BINARY_OP && node->op == '+')
        {
            stack.push_back({node->right, sign});
            stack.push_back({node->left, sign});
        }
        else if (node->type == ExprNode::Type::BINARY_OP && node->op == '-')
        {
            stack.push_back({node->right, -sign});
            stack.push_back({node->left, sign});
        }
        else if (node->type == ExprNode::Type::UNARY_OP && node->op == '+')
        {
            stack.push_back({node->operand, sign});
        }
        else if (node->type == ExprNode::Type::UNARY_OP && node->op == '-')
        {
            stack.push_back({node->operand, -sign});
        }
        else if (node->type == ExprNode::Type::NUMBER || node->type == ExprNode::Type::LABEL)
        {
            if (sign == 1)
            {
                constTerms.push_back(node);
            }
            else
            {
                std::shared_ptr<ExprNode> mul = std::make_shared<ExprNode>();
                mul->type = ExprNode::Type::BINARY_OP;
                mul->op = '*';

                std::shared_ptr<ExprNode> minusOne = std::make_shared<ExprNode>();
                minusOne->type = ExprNode::Type::NUMBER;
                minusOne->value = static_cast<uint64_t>(-1);

                mul->left = minusOne;
                mul->right = node;

                constTerms.push_back(mul);
            }
        }
        else if (node->type == ExprNode::Type::REGISTER)
        {
            if (regTerms.find(node->reg) == regTerms.end())
                regs.push_back(node->reg);

            if (sign == 1)
                regTerms[node->reg].push_back(nullptr);
            else
            {
                std::shared_ptr<ExprNode> minusOne = std::make_shared<ExprNode>();
                minusOne->type = ExprNode::Type::NUMBER;
                minusOne->value = static_cast<uint64_t>(-1);
                regTerms[node->reg].push_back(minusOne);
            }
        }
        else if (node->type == ExprNode::Type::BINARY_OP && node->op == '*')
        {
            std::shared_ptr<ExprNode> regNode = nullptr;
            std::shared_ptr<ExprNode> factor = nullptr;

            if (node->left->type == ExprNode::Type::REGISTER)
            {
                regNode = node->left;
                factor = node->right;
            }
            else if (node->right->type == ExprNode::Type::REGISTER)
            {
                regNode = node->right;
                factor = node->left;
            }
            else
                throw Exception::InternalError("Invalid multiplication in addressing mode", line, column, file.c_str());

            if (sign == -1)
            {
                std::shared_ptr<ExprNode> mul = std::make_shared<ExprNode>();
                mul->type = ExprNode::Type::BINARY_OP;
                mul->op = '*';

                std::shared_ptr<ExprNode> minusOne = std::make_shared<ExprNode>();
                minusOne->type = ExprNode::Type::NUMBER;
                minusOne->value = static_cast<uint64_t>(-1);

                mul->left = minusOne;
                mul->right = node;

                factor = mul;
            }

            if (regTerms.find(regNode->reg) == regTerms.end())
                regs.push_back(regNode->reg);

            regTerms[regNode->reg].push_back(factor);
        }
        else
        {
            throw Exception::InternalError("Invalid term in addressing mode", line, column, file.c_str());
        }
    }

    if (regs.size() > 2)
        throw Exception::InternalError("Too many different registers in addressing mode", line, column, file.c_str());

    // Register 1
    if (regs.size() >= 1)
    {
        mode.reg1 = regs[0];
        mode.has_reg1 = true;
        
        std::vector<std::shared_ptr<ExprNode>>& reg1Terms = regTerms[regs[0]];
        
        if (reg1Terms[0] == nullptr)
        {
            auto one = std::make_shared<ExprNode>();
            one->type = ExprNode::Type::NUMBER;
            one->value = 1;
            mode.scale1 = one;
        }
        else
        {
            mode.scale1 = reg1Terms[0];
        }
        
        for (size_t i = 1; i < reg1Terms.size(); ++i)
        {
            auto sum = std::make_shared<ExprNode>();
            sum->type = ExprNode::Type::BINARY_OP;
            sum->op = '+';
            sum->left = mode.scale1;
            sum->right = (reg1Terms[i] == nullptr) ? 
                std::make_shared<ExprNode>() : reg1Terms[i];
            
            if (reg1Terms[i] == nullptr)
            {
                sum->right->type = ExprNode::Type::NUMBER;
                sum->right->value = 1;
            }
            
            mode.scale1 = sum;
        }
    }

    // Register 2
    if (regs.size() == 2)
    {
        mode.reg2 = regs[1];
        mode.has_reg2 = true;
        
        std::vector<std::shared_ptr<ExprNode>>& reg2Terms = regTerms[regs[1]];
        
        if (reg2Terms[0] == nullptr)
        {
            auto one = std::make_shared<ExprNode>();
            one->type = ExprNode::Type::NUMBER;
            one->value = 1;
            mode.scale2 = one;
        }
        else
        {
            mode.scale2 = reg2Terms[0];
        }
        
        for (size_t i = 1; i < reg2Terms.size(); ++i)
        {
            auto sum = std::make_shared<ExprNode>();
            sum->type = ExprNode::Type::BINARY_OP;
            sum->op = '+';
            sum->left = mode.scale2;
            sum->right = (reg2Terms[i] == nullptr) ? 
                std::make_shared<ExprNode>() : reg2Terms[i];
            
            if (reg2Terms[i] == nullptr)
            {
                sum->right->type = ExprNode::Type::NUMBER;
                sum->right->value = 1;
            }
            
            mode.scale2 = sum;
        }
    }

    // Displacement
    if (!constTerms.empty())
    {
        mode.displacement = constTerms[0];
        for (size_t i = 1; i < constTerms.size(); ++i)
        {
            auto sum = std::make_shared<ExprNode>();
            sum->type = ExprNode::Type::BINARY_OP;
            sum->op = '+';
            sum->left = mode.displacement;
            sum->right = constTerms[i];
            mode.displacement = sum;
        }
        mode.has_displacement = true;
    }

    return mode;
}
