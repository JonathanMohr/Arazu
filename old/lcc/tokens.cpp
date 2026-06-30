#include "tokens.hpp"
#include "StringPool.hpp"

#include <cctype>
#include <cstdint>
#include <cstring>
#include <io/file.hpp>

#include <ostream>

static bool IsBasicSourceChar(char c)
{ // TODO: carriage return
    return std::isalnum(c) ||
           std::isspace(c) || // space, horizontal tab, vertical tab, form feed, carriage return, newline
           std::strchr("!\"#%&'()*+,-./:;<=>?[\\]^_{|}~", c) != nullptr;
}

void C::Tokens_Old::Print(std::ostream& out)
{
    out << "Tokens_Old:" << '\n';
    for (const Token& token : tokens)
    {
        const char* tokenKindStr = "Unknown";
        switch (token.kind)
        {
            case Token::Kind::Keyword: tokenKindStr = "Keyword"; break;
            case Token::Kind::Identifier: tokenKindStr = "Identifier"; break;
            case Token::Kind::Constant: tokenKindStr = "Constant"; break;
            case Token::Kind::String_Literal: tokenKindStr = "String-Literal"; break;
            case Token::Kind::Operator: tokenKindStr = "Operator"; break;
            case Token::Kind::Punctuator: tokenKindStr = "Punctuator"; break;
        }

        out << " [" << tokenKindStr << "] "
            "(" << token.file.str() << ':'
            << token.line << ','
            << token.column << ") "
            << token.value
            << '\n';
    }
}

C::Tokens_Old C::Tokenizer_Old::Convert(Context& context, const C::PPTokens_Old& ppTokens_Old)
{
    // TODO: Choose limit dynamically with target
    static constexpr uint64_t WideCharMax = 0xFFFFFFFF;
    static constexpr uint64_t WideCharSize = 4;
    static constexpr uint64_t CharMax = 0xFF;
    static constexpr uint64_t CharSize = 1;

    static const std::unordered_map<std::string, Tokens_Old::Token::Specific> keywords =
    {
        { "auto",     Tokens_Old::Token::Specific::Keyword_Auto     },
        { "register", Tokens_Old::Token::Specific::Keyword_Register },
        { "static",   Tokens_Old::Token::Specific::Keyword_Static   },
        { "extern",   Tokens_Old::Token::Specific::Keyword_Extern   },
        { "typedef",  Tokens_Old::Token::Specific::Keyword_Typedef  },

        { "void",     Tokens_Old::Token::Specific::Keyword_Void     },
        { "char",     Tokens_Old::Token::Specific::Keyword_Char     },
        { "short",    Tokens_Old::Token::Specific::Keyword_Short    },
        { "int",      Tokens_Old::Token::Specific::Keyword_Int      },
        { "long",     Tokens_Old::Token::Specific::Keyword_Long     },
        { "float",    Tokens_Old::Token::Specific::Keyword_Float    },
        { "double",   Tokens_Old::Token::Specific::Keyword_Double   },
        { "signed",   Tokens_Old::Token::Specific::Keyword_Signed   },
        { "unsigned", Tokens_Old::Token::Specific::Keyword_Unsigned },

        { "const",    Tokens_Old::Token::Specific::Keyword_Const    },
        { "volatile", Tokens_Old::Token::Specific::Keyword_Volatile },

        { "struct",   Tokens_Old::Token::Specific::Keyword_Struct   },
        { "union",    Tokens_Old::Token::Specific::Keyword_Union    },
        { "enum",     Tokens_Old::Token::Specific::Keyword_Enum     },

        { "if",       Tokens_Old::Token::Specific::Keyword_If       },
        { "else",     Tokens_Old::Token::Specific::Keyword_Else     },
        { "switch",   Tokens_Old::Token::Specific::Keyword_Switch   },
        { "case",     Tokens_Old::Token::Specific::Keyword_Case     },
        { "default",  Tokens_Old::Token::Specific::Keyword_Default  },
        { "for",      Tokens_Old::Token::Specific::Keyword_For      },
        { "while",    Tokens_Old::Token::Specific::Keyword_While    },
        { "do",       Tokens_Old::Token::Specific::Keyword_Do       },
        { "break",    Tokens_Old::Token::Specific::Keyword_Break    },
        { "continue", Tokens_Old::Token::Specific::Keyword_Continue },

        { "return",   Tokens_Old::Token::Specific::Keyword_Return   },
        { "goto",     Tokens_Old::Token::Specific::Keyword_Goto     },
        { "sizeof",   Tokens_Old::Token::Specific::Keyword_Sizeof   },
    };

    Tokens_Old tokens;

    for (const PPTokens_Old::Token& ppToken : ppTokens_Old.GetTokens())
    {
        switch (ppToken.kind)
        {
            case PPTokens_Old::Token::Kind::Identifier:
            {
                auto it = keywords.find(ppToken.value.str());
                if (it != keywords.end())
                    tokens.tokens.emplace_back(Tokens_Old::Token::Kind::Keyword, it->second, ppToken.value, ppToken.line, ppToken.column, ppToken.file);
                else
                    tokens.tokens.emplace_back(Tokens_Old::Token::Kind::Identifier, Tokens_Old::Token::Specific::Identifier, ppToken.value, ppToken.line, ppToken.column, ppToken.file);
                break;
            }

            case PPTokens_Old::Token::Kind::PP_Number:
            {
                const std::string& raw = ppToken.value.str();
                const uint64_t length = raw.size();

                bool isFloat = false;

                uint64_t pos = 0;
                while (pos < length && std::isdigit(raw[pos]))
                    pos++;

                if (pos < length && (raw[pos] == 'e' || raw[pos] == 'E' || raw[pos] == '.'))
                    isFloat = true;

                if (isFloat)
                {
                    if (raw[pos] == '.')
                    {
                        pos++;
                        while (pos < length && std::isdigit(raw[pos]))
                            pos++;
                    }

                    if (pos < length && (raw[pos] == 'e' || raw[pos] == 'E'))
                    {
                        pos++;
                        if (pos < length && (raw[pos] == '+' || raw[pos] == '-'))
                            pos++;

                        if (pos >= length || !std::isdigit(raw[pos]))
                            throw LCC::Exception(LCC::Exception::Type::FloatExponentNoDigits, ppToken.line, ppToken.column, ppToken.file);

                        while (pos < length && std::isdigit(raw[pos]))
                            pos++;
                    }

                    if (pos < length && (raw[pos] == 'f' || raw[pos] == 'F' || raw[pos] == 'l' || raw[pos] == 'L'))
                        pos++;

                    if (pos < length && std::isalnum(raw[pos]))
                        throw LCC::Exception(LCC::Exception::Type::InvalidFloatSuffix, ppToken.line, ppToken.column, ppToken.file);

                    tokens.tokens.emplace_back(
                        Tokens_Old::Token::Kind::Constant, Tokens_Old::Token::Specific::Constant_Float,
                        ppToken.value,
                        ppToken.line, ppToken.column, ppToken.file
                    );
                }
                else
                {
                    Tokens_Old::Token::Specific specific = Tokens_Old::Token::Specific::Constant_Integer_Decimal;

                    if (raw[0] == '0' && pos == 1 &&
                        pos < length && (raw[pos] == 'x' || raw[pos] == 'X'))
                    {
                        pos++;
                        if (pos >= length || !std::isxdigit(raw[pos]))
                            throw LCC::Exception(LCC::Exception::Type::HexConstantNoDigits, ppToken.line, ppToken.column, ppToken.file);
                        while (pos < length && std::isxdigit(raw[pos]))
                            pos++;
                        specific = Tokens_Old::Token::Specific::Constant_Integer_Hex;
                    }
                    else if (raw[0] == '0')
                    {
                        for (uint64_t i = 1; i < pos; i++)
                        {
                            if (raw[i] == '8' || raw[i] == '9')
                                throw LCC::Exception(LCC::Exception::Type::InvalidOctalDigit, ppToken.line, ppToken.column, ppToken.file);
                        }
                        specific = Tokens_Old::Token::Specific::Constant_Integer_Octal;
                    }

                    bool isUnsigned = false;
                    bool isLong = false;
                    while (pos < length && std::isalnum(raw[pos]))
                    {
                        if (raw[pos] == 'u' || raw[pos] == 'U')
                        {
                            if (isUnsigned)
                                throw LCC::Exception(LCC::Exception::Type::InvalidIntegerSuffix, ppToken.line, ppToken.column, ppToken.file);
                            isUnsigned = true;
                        }
                        else if (raw[pos] == 'l' || raw[pos] == 'L')
                        {
                            if (isLong)
                                throw LCC::Exception(LCC::Exception::Type::InvalidIntegerSuffix, ppToken.line, ppToken.column, ppToken.file);
                            isLong = true;
                        }
                        else
                            throw LCC::Exception(LCC::Exception::Type::InvalidIntegerSuffix, ppToken.line, ppToken.column, ppToken.file);
                        pos++;
                    }

                    tokens.tokens.emplace_back(
                        Tokens_Old::Token::Kind::Constant, specific,
                        ppToken.value,
                        ppToken.line, ppToken.column, ppToken.file
                    );
                }

                break;
            }

            case PPTokens_Old::Token::Kind::Character_Constant:
            case PPTokens_Old::Token::Kind::String_Literal:
            {
                const bool isChar = ppToken.kind == PPTokens_Old::Token::Kind::Character_Constant;
                const bool isWide = ppToken.specific == PPTokens_Old::Token::Specific::Character_Constant_Wide ||
                                    ppToken.specific == PPTokens_Old::Token::Specific::String_Literal_Wide;

                const LCC::Exception::Type UnterminatedErrorType = isChar ? LCC::Exception::Type::UnterminatedCharLiteral : LCC::Exception::Type::UnterminatedStringLiteral;

                const std::string& raw = ppToken.value.str();
                const uint64_t length = raw.size();
                uint64_t pos = 0;

                std::string value;
                while (pos < length)
                {
                    if (!IsBasicSourceChar(raw[pos]))
                        throw LCC::Exception(LCC::Exception::Type::InvalidSourceCharacter, ppToken.line, ppToken.column, ppToken.file);
                    
                    if (raw[pos] == '\\')
                    {
                        pos++; // Skip backslash
                        if (pos >= length || raw[pos] == '\n')
                            throw LCC::Exception(UnterminatedErrorType, ppToken.line, ppToken.column, ppToken.file);

                        switch (raw[pos])
                        {
                            case '\'': value += '\''; break;
                            case '"':  value += '\"'; break;
                            case '?':  value += '\?'; break;
                            case '\\': value += '\\'; break;
                            case 'a':  value += '\a'; break;
                            case 'b':  value += '\b'; break;
                            case 'f':  value += '\f'; break;
                            case 'n':  value += '\n'; break;
                            case 'r':  value += '\r'; break;
                            case 't':  value += '\t'; break;
                            case 'v':  value += '\v'; break;

                            case 'x':
                            {
                                if (pos + 1 >= length || !std::isxdigit(raw[pos + 1]))
                                    throw LCC::Exception(LCC::Exception::Type::HexEscapeNoDigits, ppToken.line, ppToken.column, ppToken.file);

                                uint64_t hexValue = 0;
                                while (pos + 1 < length && std::isxdigit(raw[pos + 1]))
                                {
                                    hexValue = hexValue * 16 + (std::isdigit(raw[pos + 1])
                                        ? static_cast<unsigned char>(raw[pos + 1]) - '0'
                                        : static_cast<unsigned char>(std::tolower(raw[pos + 1])) - 'a' + 10);
                                    pos++;
                                }

                                const uint64_t limit = isWide ? WideCharMax : CharMax;
                                if (hexValue > limit)
                                    context.warningManager.add(LCC::Warning(LCC::Warning::Type::HexEscapeOverflow, ppToken.line, ppToken.column, ppToken.file));

                                value += static_cast<char>(hexValue & 0xFF);
                                break;
                            }

                            case '0': case '1': case '2': case '3':
                            case '4': case '5': case '6': case '7':
                            {
                                uint64_t octalValue = static_cast<unsigned char>(raw[pos]) - '0';
                                if (pos + 1 < length && raw[pos + 1] >= '0' && raw[pos + 1] <= '7')
                                {
                                    octalValue = octalValue * 8 + (static_cast<unsigned char>(raw[pos++]) - '0');
                                    if (pos + 1 < length && raw[pos + 1] >= '0' && raw[pos + 1] <= '7')
                                        octalValue = octalValue * 8 + (static_cast<unsigned char>(raw[pos++]) - '0');
                                }

                                const uint64_t limit = isWide ? WideCharMax : CharMax;
                                if (octalValue > limit)
                                    context.warningManager.add(LCC::Warning(LCC::Warning::Type::OctalEscapeOverflow, ppToken.line, ppToken.column, ppToken.file));
                                
                                value += static_cast<char>(octalValue);
                                break;
                            }

                            default:
                                throw LCC::Exception(LCC::Exception::Type::InvalidEscapeSequence, ppToken.line, ppToken.column, ppToken.file);
                        }
                    }
                    else
                    {
                        value += raw[pos];
                    }
                    pos++;
                }

                if (isChar)
                {
                    const uint64_t charSize = isWide ? WideCharSize : CharSize;
                    if (value.size() > charSize)
                        context.warningManager.add(LCC::Warning(LCC::Warning::Type::Multichar, ppToken.line, ppToken.column, ppToken.file));

                    tokens.tokens.emplace_back(
                        Tokens_Old::Token::Kind::Constant, isWide ? Tokens_Old::Token::Specific::Constant_Character_Wide : Tokens_Old::Token::Specific::Constant_Character,
                        stringPool.GetString(value),
                        ppToken.line, ppToken.column, ppToken.file
                    );
                }
                else
                {
                    const Tokens_Old::Token::Specific specific = isWide ? Tokens_Old::Token::Specific::String_Literal_Wide : Tokens_Old::Token::Specific::String_Literal;

                    if (!tokens.tokens.empty() && tokens.tokens.back().specific == specific)
                        tokens.tokens.back().value += value;
                    else
                    {
                        tokens.tokens.emplace_back(
                            Tokens_Old::Token::Kind::String_Literal, specific,
                            stringPool.GetString(value), 
                            ppToken.line, ppToken.column, ppToken.file
                        );
                    }
                }
                break;
            }

            case PPTokens_Old::Token::Kind::Operator:
            case PPTokens_Old::Token::Kind::Punctuator:
            {
                Tokens_Old::Token::Kind kind;
                Tokens_Old::Token::Specific specific;

                switch (ppToken.specific)
                {
                    case PPTokens_Old::Token::Specific::Operator_Subscript_Open:    kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Subscript_Open;    break;
                    case PPTokens_Old::Token::Specific::Operator_Subscript_Close:   kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Subscript_Close;   break;
                    case PPTokens_Old::Token::Specific::Operator_Paren_Open:        kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Paren_Open;        break;
                    case PPTokens_Old::Token::Specific::Operator_Paren_Close:       kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Paren_Close;       break;
                    case PPTokens_Old::Token::Specific::Operator_Dot:               kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Dot;               break;
                    case PPTokens_Old::Token::Specific::Operator_Arrow:             kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Arrow;             break;
                    case PPTokens_Old::Token::Specific::Operator_Increment:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Increment;         break;
                    case PPTokens_Old::Token::Specific::Operator_Decrement:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Decrement;         break;
                    case PPTokens_Old::Token::Specific::Operator_Ampersand:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Ampersand;         break;
                    case PPTokens_Old::Token::Specific::Operator_Star:              kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Star;              break;
                    case PPTokens_Old::Token::Specific::Operator_Plus:              kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Plus;              break;
                    case PPTokens_Old::Token::Specific::Operator_Minus:             kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Minus;             break;
                    case PPTokens_Old::Token::Specific::Operator_BitwiseNot:        kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_BitwiseNot;        break;
                    case PPTokens_Old::Token::Specific::Operator_LogicalNot:        kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_LogicalNot;        break;
                    case PPTokens_Old::Token::Specific::Operator_Divide:            kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Divide;            break;
                    case PPTokens_Old::Token::Specific::Operator_Modulo:            kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Modulo;            break;
                    case PPTokens_Old::Token::Specific::Operator_ShiftLeft:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_ShiftLeft;         break;
                    case PPTokens_Old::Token::Specific::Operator_ShiftRight:        kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_ShiftRight;        break;
                    case PPTokens_Old::Token::Specific::Operator_Less:              kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Less;              break;
                    case PPTokens_Old::Token::Specific::Operator_Greater:           kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Greater;           break;
                    case PPTokens_Old::Token::Specific::Operator_LessEqual:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_LessEqual;         break;
                    case PPTokens_Old::Token::Specific::Operator_GreaterEqual:      kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_GreaterEqual;      break;
                    case PPTokens_Old::Token::Specific::Operator_Equal:             kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Equal;             break;
                    case PPTokens_Old::Token::Specific::Operator_NotEqual:          kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_NotEqual;          break;
                    case PPTokens_Old::Token::Specific::Operator_BitwiseXor:        kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_BitwiseXor;        break;
                    case PPTokens_Old::Token::Specific::Operator_BitwiseOr:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_BitwiseOr;         break;
                    case PPTokens_Old::Token::Specific::Operator_LogicalAnd:        kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_LogicalAnd;        break;
                    case PPTokens_Old::Token::Specific::Operator_LogicalOr:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_LogicalOr;         break;
                    case PPTokens_Old::Token::Specific::Operator_Question:          kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Question;          break;
                    case PPTokens_Old::Token::Specific::Operator_Colon:             kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Colon;             break;
                    case PPTokens_Old::Token::Specific::Operator_Assign:            kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Assign;            break;
                    case PPTokens_Old::Token::Specific::Operator_MulAssign:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_MulAssign;         break;
                    case PPTokens_Old::Token::Specific::Operator_DivAssign:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_DivAssign;         break;
                    case PPTokens_Old::Token::Specific::Operator_ModAssign:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_ModAssign;         break;
                    case PPTokens_Old::Token::Specific::Operator_AddAssign:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_AddAssign;         break;
                    case PPTokens_Old::Token::Specific::Operator_SubAssign:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_SubAssign;         break;
                    case PPTokens_Old::Token::Specific::Operator_ShiftLeftAssign:   kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_ShiftLeftAssign;   break;
                    case PPTokens_Old::Token::Specific::Operator_ShiftRightAssign:  kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_ShiftRightAssign;  break;
                    case PPTokens_Old::Token::Specific::Operator_AndAssign:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_AndAssign;         break;
                    case PPTokens_Old::Token::Specific::Operator_XorAssign:         kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_XorAssign;         break;
                    case PPTokens_Old::Token::Specific::Operator_OrAssign:          kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_OrAssign;          break;
                    case PPTokens_Old::Token::Specific::Operator_Comma:             kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Comma;             break;
                    case PPTokens_Old::Token::Specific::Operator_Hash:              kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_Hash;              break;
                    case PPTokens_Old::Token::Specific::Operator_HashHash:          kind = Tokens_Old::Token::Kind::Operator;   specific = Tokens_Old::Token::Specific::Operator_HashHash;          break;
                    case PPTokens_Old::Token::Specific::Punctuator_Brace_Open:      kind = Tokens_Old::Token::Kind::Punctuator; specific = Tokens_Old::Token::Specific::Punctuator_Brace_Open;      break;
                    case PPTokens_Old::Token::Specific::Punctuator_Brace_Close:     kind = Tokens_Old::Token::Kind::Punctuator; specific = Tokens_Old::Token::Specific::Punctuator_Brace_Close;     break;
                    case PPTokens_Old::Token::Specific::Punctuator_Semicolon:       kind = Tokens_Old::Token::Kind::Punctuator; specific = Tokens_Old::Token::Specific::Punctuator_Semicolon;       break;
                    case PPTokens_Old::Token::Specific::Punctuator_Ellipsis:        kind = Tokens_Old::Token::Kind::Punctuator; specific = Tokens_Old::Token::Specific::Punctuator_Ellipsis;        break;

                    default:
                        throw LCC::Exception(LCC::Exception::Type::Internal_InvalidOperatorSpecificKind, ppToken.line, ppToken.column, ppToken.file);
                }

                tokens.tokens.emplace_back(kind, specific, ppToken.value, ppToken.line, ppToken.column, ppToken.file);
                break;
            }

            case PPTokens_Old::Token::Kind::Newline:
                break;

            case PPTokens_Old::Token::Kind::Header_Name:
                throw LCC::Exception(LCC::Exception::Type::Internal_PPHeaderNameInConvert, ppToken.line, ppToken.column, ppToken.file);

            case PPTokens_Old::Token::Kind::PP_Other:
            default:
                throw LCC::Exception(LCC::Exception::Type::UnexpectedCharacter, ppToken.line, ppToken.column, ppToken.file);
        }
    }

    return tokens;
}
