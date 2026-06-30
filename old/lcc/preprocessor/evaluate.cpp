#include "preprocessor.hpp"

#include <tokens.hpp>

int64_t C::Preprocessor::EvaluateConstantExpression(
    const std::vector<PPTokens_Old::Token>& tokens,
    std::vector<PPTokens_Old::Token>::size_type& pos,
    RobinHoodMap<StringPool::String, Macro>& macros,
    Context& context,
    TokenBufferPool& tokenBufferPool
)
{
    if (pos >= tokens.size() || tokens[pos].specific == PPTokens_Old::Token::Specific::Newline)
        throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression, tokens[pos].line, tokens[pos].column, tokens[pos].file);

    std::vector<PPTokens_Old::Token> expanded;
    PPTokens_Old expandedPP;
    while (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
        ExpandToken(tokens, pos, expandedPP, macros, nullptr, context, tokenBufferPool, true);
    expanded = expandedPP.GetTokens();

    std::vector<PPTokens_Old::Token>::size_type epos = 0;
    int64_t result = ParseConditional(expanded, epos, macros, context);

    if (epos < expanded.size())
        throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression, expanded[epos].line, expanded[epos].column, expanded[epos].file);

    return result;
}

#define ParseFunction(name) int64_t C::Preprocessor::name(                              \
                                const std::vector<PPTokens_Old::Token>& tokens,             \
                                std::vector<PPTokens_Old::Token>::size_type& pos,           \
                                const RobinHoodMap<StringPool::String, Macro>& macros,  \
                                Context& context                                        \
                            )

ParseFunction(ParseConditional)
{
    int64_t value = ParseLogicalOr(tokens, pos, macros, context);

    if (pos < tokens.size() && tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Question)
    {
        pos++; // skip ?

        int64_t thenValue = ParseConditional(tokens, pos, macros, context);

        if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Colon)
            throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression,
                tokens[pos - 1].line, tokens[pos - 1].column, tokens[pos - 1].file);

        pos++; // skip :

        int64_t elseValue = ParseConditional(tokens, pos, macros, context);

        return value ? thenValue : elseValue;
    }

    return value;
}

ParseFunction(ParseLogicalOr)
{
    int64_t value = ParseLogicalAnd(tokens, pos, macros, context);

    while (pos < tokens.size() && tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_LogicalOr)
    {
        pos++;
        int64_t right = ParseLogicalAnd(tokens, pos, macros, context);
        value = value || right;
    }

    return value;
}

ParseFunction(ParseLogicalAnd)
{
    int64_t value = ParseBinaryOr(tokens, pos, macros, context);

    while (pos < tokens.size() && tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_LogicalAnd)
    {
        pos++;
        int64_t right = ParseBinaryOr(tokens, pos, macros, context);
        value = value && right;
    }

    return value;
}

ParseFunction(ParseBinaryOr)
{
    int64_t value = ParseBinaryXor(tokens, pos, macros, context);

    while (pos < tokens.size() && tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_BitwiseOr)
    {
        pos++;
        int64_t right = ParseBinaryXor(tokens, pos, macros, context);
        value = value | right;
    }

    return value;
}

ParseFunction(ParseBinaryXor)
{
    int64_t value = ParseBinaryAnd(tokens, pos, macros, context);

    while (pos < tokens.size() && tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_BitwiseXor)
    {
        pos++;
        int64_t right = ParseBinaryAnd(tokens, pos, macros, context);
        value = value ^ right;
    }

    return value;
}

ParseFunction(ParseBinaryAnd)
{
    int64_t value = ParseEquality(tokens, pos, macros, context);

    while (pos < tokens.size() && tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Ampersand)
    {
        pos++;
        int64_t right = ParseEquality(tokens, pos, macros, context);
        value = value & right;
    }

    return value;
}

ParseFunction(ParseEquality)
{
    int64_t value = ParseRelational(tokens, pos, macros, context);

    while (pos < tokens.size() &&
           (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Equal ||
            tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_NotEqual))
    {
        const bool isEqual = tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Equal;
        pos++;
        int64_t right = ParseRelational(tokens, pos, macros, context);
        value = isEqual ? (value == right) : (value != right);
    }

    return value;
}

ParseFunction(ParseRelational)
{
    int64_t value = ParseShift(tokens, pos, macros, context);

    while (pos < tokens.size() &&
           (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Less ||
            tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Greater ||
            tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_LessEqual ||
            tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_GreaterEqual))
    {
        const PPTokens_Old::Token::Specific op = tokens[pos].specific;
        pos++;
        int64_t right = ParseShift(tokens, pos, macros, context);

        switch (op)
        {
            case PPTokens_Old::Token::Specific::Operator_Less:         value = value < right;  break;
            case PPTokens_Old::Token::Specific::Operator_Greater:      value = value > right;  break;
            case PPTokens_Old::Token::Specific::Operator_LessEqual:    value = value <= right; break;
            case PPTokens_Old::Token::Specific::Operator_GreaterEqual: value = value >= right; break;
            default: break; // Does not happen, but my IDE wants me to put it here
        }
    }

    return value;
}

ParseFunction(ParseShift)
{
    int64_t value = ParseAddSub(tokens, pos, macros, context);

    while (pos < tokens.size() &&
           (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_ShiftLeft ||
            tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_ShiftRight))
    {
        const bool isLeft = tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_ShiftLeft;
        pos++;
        int64_t right = ParseAddSub(tokens, pos, macros, context);
        value = isLeft ? (value << right) : (value >> right);
    }

    return value;
}

ParseFunction(ParseAddSub)
{
    int64_t value = ParseMulDiv(tokens, pos, macros, context);

    while (pos < tokens.size() &&
           (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Plus ||
            tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Minus))
    {
        const bool isAdd = tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Plus;
        pos++;
        int64_t right = ParseMulDiv(tokens, pos, macros, context);
        value = isAdd ? (value + right) : (value - right);
    }

    return value;
}

ParseFunction(ParseMulDiv)
{
    int64_t value = ParseUnary(tokens, pos, macros, context);

    while (pos < tokens.size() &&
           (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Star ||
            tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Divide ||
            tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Modulo))
    {
        const PPTokens_Old::Token::Specific op = tokens[pos].specific;
        pos++;
        int64_t right = ParseUnary(tokens, pos, macros, context);

        if (op == PPTokens_Old::Token::Specific::Operator_Star)
            value = value * right;
        else if (op == PPTokens_Old::Token::Specific::Operator_Divide)
        {
            if (right == 0)
                throw LCC::Exception(LCC::Exception::Type::DivisionByZero,
                    tokens[pos - 1].line, tokens[pos - 1].column, tokens[pos - 1].file);
            value = value / right;
        }
        else
        {
            if (right == 0)
                throw LCC::Exception(LCC::Exception::Type::DivisionByZero,
                    tokens[pos - 1].line, tokens[pos - 1].column, tokens[pos - 1].file);
            value = value % right;
        }
    }

    return value;
}

ParseFunction(ParseUnary)
{
    if (pos < tokens.size())
    {
        if (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Plus)
        {
            pos++;
            return ParseUnary(tokens, pos, macros, context);
        }
        if (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Minus)
        {
            pos++;
            return -ParseUnary(tokens, pos, macros, context);
        }
        if (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_LogicalNot)
        {
            pos++;
            return !ParseUnary(tokens, pos, macros, context);
        }
        if (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_BitwiseNot)
        {
            pos++;
            return ~ParseUnary(tokens, pos, macros, context);
        }
    }

    return ParsePrimary(tokens, pos, macros, context);
}

ParseFunction(ParsePrimary)
{
    if (pos >= tokens.size())
        throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression,
            tokens[tokens.size() - 1].line, tokens[tokens.size() - 1].column, tokens[tokens.size() - 1].file);

    const PPTokens_Old::Token& token = tokens[pos];

    // Integer
    if (token.kind == PPTokens_Old::Token::Kind::PP_Number)
    {
        pos++;

        const std::string s = token.value.str();
        uint64_t idx = 0;
        int base = 10;

        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        {
            base = 16;
            idx = 2;
        }
        else if (s.size() > 1 && s[0] == '0')
        {
            base = 8;
            idx = 1;
        }

        // TODO: Do not use stoll
        return std::stoll(s.substr(idx), nullptr, base);
    }
    else if (token.kind == PPTokens_Old::Token::Kind::Character_Constant)
    {
        // TODO
    }

    // defined(NAME) or defined NAME
    if (token.specific == PPTokens_Old::Token::Specific::Identifier && token.value == "defined")
    {
        pos++;
        bool hasParen = false;

        if (pos < tokens.size() && tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Paren_Open)
        {
            hasParen = true;
            pos++;
        }

        if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Identifier)
            throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression, // TODO: Could be more specific
                token.line, token.column, token.file);

        const StringPool::String name = tokens[pos].value;
        pos++;

        if (hasParen)
        {
            if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Paren_Close)
                throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression, // TODO: Could be more specific
                    token.line, token.column, token.file);
            pos++;
        }

        return macros.find(name) ? 1 : 0;
    }

    // Undefined identifier -> 0
    if (token.specific == PPTokens_Old::Token::Specific::Identifier)
    {
        pos++;
        return 0;
    }

    // Parenthesized expression
    if (token.specific == PPTokens_Old::Token::Specific::Operator_Paren_Open)
    {
        pos++;
        int64_t value = ParseConditional(tokens, pos, macros, context);

        if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Paren_Close)
            throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression, // TODO: Could be more specific
                token.line, token.column, token.file);
        pos++;
        return value;
    }

    throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression, token.line, token.column, token.file);
}
