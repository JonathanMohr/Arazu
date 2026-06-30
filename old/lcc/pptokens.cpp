#include "StringPool.hpp"
#include "tokens.hpp"

#include <cctype>
#include <cstdint>
#include <io/file.hpp>
#include <string>

std::optional<C::PPTokens_Old::Token> C::PPTokens_Old::GetOperator(const std::string& str, uint64_t lineNumber, uint64_t& pos, StringPool::String file, uint64_t leadingWhitespaces)
{
    const uint64_t startPos = pos;
    auto make = [&](auto kind, auto specific, const std::string& text)
        -> std::optional<Token>
    {
        return Token(kind, specific, stringPool.GetString(text), lineNumber, startPos + 1, file, leadingWhitespaces);
    };

    const uint64_t length = str.size();

    switch (str[pos])
    {
        case '[': pos++; return make(Token::Kind::Operator, Token::Specific::Operator_Subscript_Open, "[");
        case ']': pos++; return make(Token::Kind::Operator, Token::Specific::Operator_Subscript_Close, "]");
        case '(': pos++; return make(Token::Kind::Operator, Token::Specific::Operator_Paren_Open, "(");
        case ')': pos++; return make(Token::Kind::Operator, Token::Specific::Operator_Paren_Close, ")");
        case '{': pos++; return make(Token::Kind::Punctuator, Token::Specific::Punctuator_Brace_Open, "{");
        case '}': pos++; return make(Token::Kind::Punctuator, Token::Specific::Punctuator_Brace_Close, "}");
        case ';': pos++; return make(Token::Kind::Punctuator, Token::Specific::Punctuator_Semicolon, ";");
        case '~': pos++; return make(Token::Kind::Operator, Token::Specific::Operator_BitwiseNot, "~");
        case '?': pos++; return make(Token::Kind::Operator, Token::Specific::Operator_Question, "?");
        case ',': pos++; return make(Token::Kind::Operator, Token::Specific::Operator_Comma, ",");

        case '.':
            if (pos + 2 < length && str[pos + 1] == '.' && str[pos + 2] == '.')
            {
                pos += 3;
                return make(Token::Kind::Punctuator, Token::Specific::Punctuator_Ellipsis, "...");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Dot, ".");

        case '+':
            if (pos + 1 < length && str[pos + 1] == '+')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_Increment, "++");
            }
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_AddAssign, "+=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Plus, "+");

        case '-':
            if (pos + 1 < length && str[pos + 1] == '-')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_Decrement, "--");
            }
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_SubAssign, "-=");
            }
            if (pos + 1 < length && str[pos + 1] == '>')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_Arrow, "->");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Minus, "-");

        case '*':
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_MulAssign, "*=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Star, "*");

        case '/':
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_DivAssign, "/=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Divide, "/");

        case '%':
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_ModAssign, "%=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Modulo, "%");

        case '=':
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_Equal, "==");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Assign, "=");

        case '!':
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_NotEqual, "!=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_LogicalNot, "!");

        case '&':
            if (pos + 1 < length && str[pos + 1] == '&')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_LogicalAnd, "&&");
            }
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_AndAssign, "&=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Ampersand, "&");

        case '|':
            if (pos + 1 < length && str[pos + 1] == '|')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_LogicalOr, "||");
            }
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_OrAssign, "|=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_BitwiseOr, "|");

        case '^':
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_XorAssign, "^=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_BitwiseXor, "^");

        case '<':
            if (pos + 1 < length && str[pos + 1] == '<')
            {
                if (pos + 2 < length && str[pos + 2] == '=')
                {
                    pos += 3;
                    return make(Token::Kind::Operator, Token::Specific::Operator_ShiftLeftAssign, "<<=");
                }
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_ShiftLeft, "<<");
            }
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_LessEqual, "<=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Less, "<");

        case '>':
            if (pos + 1 < length && str[pos + 1] == '>')
            {
                if (pos + 2 < length && str[pos + 2] == '=')
                {
                    pos += 3;
                    return make(Token::Kind::Operator, Token::Specific::Operator_ShiftRightAssign, ">>=");
                }
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_ShiftRight, ">>");
            }
            if (pos + 1 < length && str[pos + 1] == '=')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_GreaterEqual, ">=");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Greater, ">");

        case ':':
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Colon, ":");

        case '#':
            if (pos + 1 < length && str[pos + 1] == '#')
            {
                pos += 2;
                return make(Token::Kind::Operator, Token::Specific::Operator_HashHash, "##");
            }
            pos++;
            return make(Token::Kind::Operator, Token::Specific::Operator_Hash, "#");

        default:
            return std::nullopt;
    }
}

static std::string replaceTrigraphs(std::string rawLine)
{
    if (rawLine.find("??") == std::string::npos)
        return rawLine;

    uint64_t rawPos = 0;
    const uint64_t rawLength = rawLine.length();

    std::string line;
    line.reserve(rawLength);

    while (rawPos < rawLength)
    {
        // TODO: Column

        if (rawLine[rawPos] == '?' && rawPos + 2 < rawLength && rawLine[rawPos + 1] == '?')
        {
            char actual = 0;
            switch (rawLine[rawPos + 2])
            {
                case '=':  actual = '#';  break;
                case '(':  actual = '[';  break;
                case '/':  actual = '\\'; break;
                case ')':  actual = ']';  break;
                case '\'': actual = '^';  break;
                case '<':  actual = '{';  break;
                case '!':  actual = '|';  break;
                case '>':  actual = '}';  break;
                case '-':  actual = '~';  break;
                default: break;
            }

            if (actual != 0)
            {
                line += actual;
                rawPos += 3;
                continue;
            }
        }

        line += rawLine[rawPos];
        rawPos++;
    }

    return line;
}

void C::PPTokens_Old::Print(std::ostream& out)
{
    out << "Preprocessing-Tokens_Old:" << '\n';
    for (const Token& token : tokens)
    {
        const char* tokenKindStr = "Unknown";
        switch (token.kind)
        {
            case Token::Kind::Header_Name: tokenKindStr = "Header-Name"; break;
            case Token::Kind::Identifier: tokenKindStr = "Identifier"; break;
            case Token::Kind::PP_Number: tokenKindStr = "PP-Number"; break;
            case Token::Kind::Character_Constant: tokenKindStr = "Character-Constant"; break;
            case Token::Kind::String_Literal: tokenKindStr = "String-Literal"; break;
            case Token::Kind::Operator: tokenKindStr = "Operator"; break;
            case Token::Kind::Punctuator: tokenKindStr = "Punctuator"; break;
            case Token::Kind::PP_Other: tokenKindStr = "Other"; break;
            case Token::Kind::Newline: tokenKindStr = "Newline"; break;
        }

        out << " [" << tokenKindStr << "] ("
            << token.file.str() << ':'
            << token.line << ','
            << token.column << "|"
            << token.leadingWhitespaces << ')';
        if (token.kind != Token::Kind::Newline)
            out << ' ' << token.value;
        out << '\n';
    }
}

C::PPTokens_Old C::Tokenizer_Old::Tokenize(Context& context, const std::vector<std::string> &filenames)
{
    bool warnedWithNonStandardSyntaxForLineDefinition = false;
    PPTokens_Old tokens;

    tokens.tokens.reserve(4096); // TODO: Currently just random

    for (const std::string& filename : filenames)
        Tokenize(context, filename.c_str(), tokens, warnedWithNonStandardSyntaxForLineDefinition);

    return tokens;
}

void C::Tokenizer_Old::Tokenize(Context& context, const char* filename, C::PPTokens_Old& tokens, bool& warnedWithNonStandardSyntaxForLineDefinition)
{
    InputStream input = openInputStream(filename);

    StringPool::String file = stringPool.GetString(filename);
    std::string rawLine;

    uint64_t lineNumber = 0;

    uint64_t commentLine;
    uint64_t commentColumn;
    bool inComment = false;

    std::string nextLine;

    while (std::getline(*input, rawLine))
    {
        std::string line = replaceTrigraphs(std::move(rawLine));

        uint64_t lineExtra = 0;
        lineNumber++;

        while (!line.empty() && line.back() == '\\')
        {
            nextLine.clear();
            if (!std::getline(*input, nextLine))
                throw LCC::Exception(LCC::Exception::Type::BackslashAtEndOfFile, lineNumber, line.size(), file);

            lineExtra++;
            line.pop_back();
            line += replaceTrigraphs(std::move(nextLine));
        }

        uint64_t tmpPos = 0;
        while (tmpPos < line.size() && std::isspace(line[tmpPos]))
            tmpPos++;

        if (tmpPos < line.size() && line[tmpPos] == '#')
        {
            const uint64_t length = line.size();

            tmpPos++;

            while (tmpPos < length && std::isspace(line[tmpPos]))
                tmpPos++;

            if (tmpPos < length && std::isdigit(line[tmpPos]))
            {
                if (!warnedWithNonStandardSyntaxForLineDefinition)
                {
                    context.warningManager.add(LCC::Warning(LCC::Warning::Type::NonStandardSyntaxForLineDefinition, lineNumber, 1, file));
                    warnedWithNonStandardSyntaxForLineDefinition = true;
                }

                uint64_t newLine = 0;
                while (std::isdigit(line[tmpPos]))
                {
                    newLine = static_cast<uint64_t>(line[tmpPos] - '0');
                    tmpPos++;
                }

                while (tmpPos < length && std::isspace(line[tmpPos]))
                    tmpPos++;

                if (tmpPos < length)
                {
                    if (line[tmpPos] != '"')
                        throw LCC::Exception(LCC::Exception::Type::SetFileNotString, lineNumber, line.size(), file);

                    const bool isWide = line[tmpPos] == 'L';
                    if (isWide) tmpPos++;
                    tmpPos++; // Skip opening "

                    if (tmpPos >= length || line[tmpPos] == '\n')
                        throw LCC::Exception(LCC::Exception::Type::UnterminatedStringLiteral, lineNumber, tmpPos + 1, file);

                    std::string value;
                    while (tmpPos < length && line[tmpPos] != '"')
                    {
                        if (line[tmpPos] == '\n')
                            throw LCC::Exception(LCC::Exception::Type::UnterminatedStringLiteral, lineNumber, tmpPos + 1, file);

                        if (line[tmpPos] == '\\')
                        {
                            if (tmpPos + 1 >= length)
                                throw LCC::Exception(LCC::Exception::Type::UnterminatedStringLiteral, lineNumber, tmpPos + 1, file);

                            switch (line[tmpPos + 1])
                            {
                                case '\'': value += '\''; break;
                                case '"': value += '\"'; break;
                                case '?': value += '\?'; break;
                                case '\\': value += '\\'; break;

                                default:
                                    throw LCC::Exception(LCC::Exception::Type::InvalidEscapeSequence, lineNumber, tmpPos + 1, file);
                            }

                            tmpPos += 2;
                        }
                        else
                        {
                            value += line[tmpPos];
                            tmpPos++;
                        }
                    }

                    if (tmpPos >= length || line[tmpPos] != '"')
                        throw LCC::Exception(LCC::Exception::Type::UnterminatedStringLiteral, lineNumber, tmpPos + 1, file);

                    tmpPos++; // Skip closing "

                    file = stringPool.GetString(value);
                }

                lineNumber = newLine - 1;

                continue;
            }
        }

        uint64_t pos = 0;
        const uint64_t length = line.size();

        uint64_t leadingWhitespaces = 0;

        while (pos < length)
        {
            if (inComment)
            {
                if (line[pos] == '*' && pos + 1 < length && line[pos + 1] == '/')
                {
                    leadingWhitespaces++;
                    pos += 2;
                    inComment = false;
                }
                else
                    pos++;
                continue;
            }

            if (std::isspace(line[pos]))
            {
                leadingWhitespaces++;
                pos++;
                continue;
            }

            if (line[pos] == '/' && pos + 1 < length)
            {
                if (line[pos + 1] == '/')
                {
                    // TODO: C99 only, not valid in C89
                    pos = length;
                    continue;
                }
                if (line[pos + 1] == '*')
                {
                    inComment = true;
                    commentLine = lineNumber;
                    commentColumn = pos;

                    pos += 2;
                    continue;
                }
            }

            // PPNumber
            if (std::isdigit(line[pos]) || (line[pos] == '.' && pos + 1 < length && std::isdigit(line[pos + 1])))
            {
                const uint64_t start = pos;
                while (pos < length && (std::isalnum(line[pos]) || line[pos] == '.' || line[pos] == 'e' || line[pos] == 'E'))
                {
                    pos++;
                    if (pos < length && (line[pos - 1] == 'e' || line[pos - 1] == 'E') && (line[pos] == '+' || line[pos] == '-'))
                        pos++;
                }

                tokens.tokens.emplace_back(
                    PPTokens_Old::Token::Kind::PP_Number,
                    PPTokens_Old::Token::Specific::PP_Number,
                    stringPool.GetString(line.substr(start, pos - start)),
                    lineNumber, start + 1, file,
                    leadingWhitespaces
                );
                leadingWhitespaces = 0;
            }

            // Character-Constant
            else if (line[pos] == '\'' || (line[pos] == 'L' && pos + 1 < length && line[pos + 1] == '\''))
            {
                const uint64_t cStart = pos;

                const bool isWide = line[pos] == 'L';
                if (isWide) pos++;
                pos++; // Skip opening '

                if (pos >= length || line[pos] == '\n')
                    throw LCC::Exception(LCC::Exception::Type::UnterminatedCharLiteral, lineNumber, pos + 1, file);

                if (line[pos] == '\'')
                    throw LCC::Exception(LCC::Exception::Type::EmptyCharLiteral, lineNumber, pos + 1, file);

                std::string value;
                while (pos < length && line[pos] != '\'')
                {
                    if (line[pos] == '\n')
                        throw LCC::Exception(LCC::Exception::Type::UnterminatedCharLiteral, lineNumber, pos + 1, file);

                    if (line[pos] == '\\' && pos + 1 < length)
                    {
                        value += '\\';
                        value += line[pos + 1];
                        pos += 2;
                    }
                    else
                    {
                        value += line[pos];
                        pos++;
                    }
                }

                if (pos >= length || line[pos] != '\'')
                    throw LCC::Exception(LCC::Exception::Type::UnterminatedCharLiteral, lineNumber, pos + 1, file);

                pos++; // Skip closing '

                PPTokens_Old::Token token;
                token.line = lineNumber;
                token.column = cStart + 1;
                token.file = file;
                token.value = stringPool.GetString(value);

                token.leadingWhitespaces = leadingWhitespaces;

                token.kind = PPTokens_Old::Token::Kind::Character_Constant;
                token.specific = isWide ? PPTokens_Old::Token::Specific::Character_Constant_Wide : PPTokens_Old::Token::Specific::Character_Constant;

                tokens.tokens.push_back(token);

                leadingWhitespaces = 0;
            }

            // String literal
            else if (line[pos] == '"' || (line[pos] == 'L' && pos + 1 < length && line[pos + 1] == '"'))
            {
                const uint64_t sStart = pos;

                const bool isWide = line[pos] == 'L';
                if (isWide) pos++;
                pos++; // Skip opening "

                if (pos >= length || line[pos] == '\n')
                    throw LCC::Exception(LCC::Exception::Type::UnterminatedStringLiteral, lineNumber, pos + 1, file);

                std::string value;
                while (pos < length && line[pos] != '"')
                {
                    if (line[pos] == '\n')
                        throw LCC::Exception(LCC::Exception::Type::UnterminatedStringLiteral, lineNumber, pos + 1, file);

                    if (line[pos] == '\\' && pos + 1 < length)
                    {
                        value += '\\';
                        value += line[pos + 1];
                        pos += 2;
                    }
                    else
                    {
                        value += line[pos];
                        pos++;
                    }
                }

                if (pos >= length || line[pos] != '"')
                    throw LCC::Exception(LCC::Exception::Type::UnterminatedStringLiteral, lineNumber, pos + 1, file);

                pos++; // Skip closing "

                PPTokens_Old::Token token;
                token.line = lineNumber;
                token.column = sStart + 1;
                token.file = file;
                token.value = stringPool.GetString(value);

                token.leadingWhitespaces = leadingWhitespaces;

                token.kind = PPTokens_Old::Token::Kind::String_Literal;
                token.specific = isWide ? PPTokens_Old::Token::Specific::String_Literal_Wide : PPTokens_Old::Token::Specific::String_Literal;

                tokens.tokens.push_back(token);

                leadingWhitespaces = 0;
            }

            // Keyword
            // Identifier
            else if (std::isalpha(line[pos]) || line[pos] == '_')
            {
                uint64_t start = pos;
                while (pos < length && (std::isalnum(line[pos]) || line[pos] == '_'))
                    pos++;

                tokens.tokens.emplace_back(
                    PPTokens_Old::Token::Kind::Identifier, PPTokens_Old::Token::Specific::Identifier,
                    stringPool.GetString(std::string_view(line).substr(start, pos - start)),
                    lineNumber, start + 1, file,
                    leadingWhitespaces
                );
                leadingWhitespaces = 0;
            }

            // Operator
            // Punctuator
            // Other
            else
            {
                if (auto tok = PPTokens_Old::GetOperator(line, lineNumber, pos, file, leadingWhitespaces); tok.has_value())
                {
                    tokens.tokens.emplace_back(tok.value());
                }
                else
                {
                    tokens.tokens.emplace_back(
                        PPTokens_Old::Token::Kind::PP_Other,
                        PPTokens_Old::Token::Specific::PP_Other,
                        stringPool.GetString(line[pos]),
                        lineNumber,
                        pos + 1,
                        file,
                        leadingWhitespaces
                    );
                    pos++;
                }
                leadingWhitespaces = 0;
            }
        }

        for (uint64_t i = 0; i < lineExtra; i++)
        {
            lineNumber++;
            tokens.tokens.emplace_back(PPTokens_Old::Token::Kind::Newline, PPTokens_Old::Token::Specific::Newline, stringPool.GetString('\n'), lineNumber, line.length(), file, leadingWhitespaces);
        }

        tokens.tokens.emplace_back(PPTokens_Old::Token::Kind::Newline, PPTokens_Old::Token::Specific::Newline, stringPool.GetString('\n'), lineNumber, line.length(), file, leadingWhitespaces);
    }

    if (inComment)
        throw LCC::Exception(LCC::Exception::Type::UnterminatedBlockComment, commentLine, commentColumn + 1, file);
}

void C::Tokenizer_Old::Serialize(Context& context, const PPTokens_Old &ppTokens_Old, std::ostream& out)
{
    StringPool::String currentFile;

    uint64_t currentLine = 1;

    auto putFile = [&out, &currentFile, &currentLine](StringPool::String file, uint64_t line) {
        out << "# " << line
            << " \"";

        for (char c : file.str())
        {
            if (c == '\\') out << "\\\\";
            else           out << c;
        }

        out << "\"\n";

        currentLine = line;
        currentFile = file;
    };

    if (!ppTokens_Old.GetTokens().empty()) putFile(ppTokens_Old.GetTokens()[0].file, ppTokens_Old.GetTokens()[0].line);
    for (const PPTokens_Old::Token& token : ppTokens_Old.GetTokens())
    {
        if (token.specific == PPTokens_Old::Token::Specific::Newline)
            continue;

        if (token.line != currentLine || currentFile != token.file)
        {
            const uint64_t threshold = 25; // TODO: Actually calculate

            if (token.line > currentLine && (token.line - currentLine < threshold) && currentFile == token.file)
            {
                while (token.line > currentLine)
                {
                    out << '\n';
                    currentLine++;
                }
            }
            else
            {
                out << '\n';
                putFile(token.file, token.line);
            }
        }
        
        if (token.line < currentLine)
        {
            // TODO
        }

        if (token.leadingWhitespaces > 0)
        {
            for (uint64_t i = 0; i < token.leadingWhitespaces; i++) out << ' ';
        }

        switch (token.kind)
        {
            case PPTokens_Old::Token::Kind::Character_Constant:
                out << (token.specific == PPTokens_Old::Token::Specific::Character_Constant_Wide ? "L'" : "'")
                    << token.value.str()
                    << '\'';
                break;

            case PPTokens_Old::Token::Kind::String_Literal:
                out << (token.specific == PPTokens_Old::Token::Specific::String_Literal_Wide ? "L\"" : "\"")
                    << token.value.str()
                    << '"';
                break;

            case PPTokens_Old::Token::Kind::Newline:
                break;

            default:
                out << token.value.str();
                break;
        }
    }

    out << '\n';
}
