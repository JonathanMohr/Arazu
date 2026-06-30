#include "parser.hpp"

#include <util/string.hpp>
#include <io/file.hpp>

void Assembler::Tokenizer::Tokenize(const Context& context, const char* filename, Tokens& tokens)
{
    InputStream input = openInputStream(filename);

    StringPool::String file = context.stringPool->GetString(filename);
    std::string line;

    uint64_t lineNumber = 0;
    uint64_t lineIncrease = 1;

    // TODO: hardcoded
    const char commentStart = ';';

    while (std::getline(*input, line))
    {
        lineNumber += lineIncrease;
        uint64_t pos = 0;
        uint64_t length = line.size();

        uint64_t spaceCountLine = 0;
        std::string trimmedLine = trim(line, spaceCountLine);

        if (!trimmedLine.empty() && trimmedLine[0] == commentStart) continue;

        uint64_t commentPos = line.find(';');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
            length = line.size();
        }

        if (trimmedLine.find("%line") == 0)
        {
            std::string rest = trim(trimmedLine.substr(5));
            uint64_t plusPos = rest.find('+');
            uint64_t spacePos = rest.find(' ');

            if (plusPos != std::string::npos)
                lineIncrease = std::stoull(trim(rest.substr(plusPos + 1, spacePos - plusPos - 1)));

            lineNumber = std::stoull(trim(rest.substr(0, plusPos))) - lineIncrease;

            if (spacePos != std::string::npos)
                file = context.stringPool->GetString(trim(rest.substr(spacePos + 1)));

            continue;
        }

        while (pos < length)
        {
            uint64_t startPos = pos;
            uint64_t tokenSpaceCount = 0;

            std::string rest = trim(line.substr(pos), tokenSpaceCount);
            if (rest.empty()) break;

            pos += tokenSpaceCount;
            startPos = pos;

            // ,
            if (line[pos] == ',')
            {
                tokens.addToken(
                    Tokens::Type::Comma,
                    context.stringPool->GetString(","),
                    lineNumber, pos + 1,
                    file, tokenSpaceCount
                );
                pos++;
            }
            // ; or :
            else if (line[pos] == ';' || line[pos] == ':')
            {
                tokens.addToken(
                    Tokens::Type::Punctuation,
                    context.stringPool->GetString(line[pos]),
                    lineNumber, pos,
                    file, tokenSpaceCount
                );
                pos++;
            }

            // +,-,*,/
            else if (line[pos] == '+' || line[pos] == '-' || line[pos] == '*' || line[pos] == '/' || line[pos] == '%')
            {
                tokens.addToken(
                    Tokens::Type::Operator,
                    context.stringPool->GetString(line[pos]),
                    lineNumber, pos,
                    file, tokenSpaceCount
                );
                pos++;
            }

            // Bracket
            else if (line[pos] == '(' || line[pos] == ')' ||
                     line[pos] == '[' || line[pos] == ']' ||
                     line[pos] == '{' || line[pos] == '}')
            {
                tokens.addToken(
                    Tokens::Type::Bracket,
                    context.stringPool->GetString(line[pos]),
                    lineNumber, pos + 1,
                    file, tokenSpaceCount
                );
                pos++;
            }

            // Strings
            else if (line[pos] == '"')
            {
                pos++;  // skip opening "
                startPos = pos;
                std::string value;
                while (pos < length)
                {
                    if (line[pos] == '\\')
                    {
                        pos++;
                        switch(line[pos])
                        {
                            case '\\': value.push_back('\\'); pos++; break;
                            case '"': value.push_back('"'); pos++; break;
                            case '\'': value = '\''; pos++; break;
                            case 'n': value = '\n'; pos++; break;
                            // TODO: add more

                            default: throw Exception::SyntaxError("Unknown escape character", lineNumber, pos, file.c_str());
                        }
                    }
                    else if (line[pos] == '"')
                    {
                        break;
                    }
                    else
                    {
                        value.push_back(line[pos]);
                        pos++;
                    }
                }

                tokens.addToken(
                    Tokens::Type::String,
                    context.stringPool->GetString(value),
                    lineNumber, startPos,
                    file, tokenSpaceCount
                );

                if (pos < length && line[pos] == '"')
                    pos++; // skip closing "
            }
            // Characters
            else if (line[pos] == '\'')
            {
                pos++;  // skip opening '
                startPos = pos;
                char value;
                if (line[pos] == '\\')
                {
                    pos++;
                    if (pos >= line.length())
                        throw Exception::SyntaxError("Unexpected end of line after escape character", lineNumber, pos, file.c_str());

                    // TODO: one function only
                    switch(line[pos])
                    {
                        case '\\': value = '\\'; break;
                        case '"': value = '"'; break;
                        case '\'': value = '\''; break;
                        case 'n': value = '\n';break;
                        // TODO: add more

                        default: throw Exception::SyntaxError("Unknown escape character", lineNumber, pos, file.c_str());
                    }
                }
                else
                {
                    value = line[pos];
                }

                pos++;

                if (pos >= line.length() || line[pos] != '\'')
                {
                    throw Exception::SyntaxError("Expected closing '", lineNumber, pos, file.c_str());
                }

                tokens.addToken(
                    Tokens::Type::Character,
                    context.stringPool->GetString(value),
                    lineNumber, startPos,
                    file, tokenSpaceCount
                );

                pos++; // skip closing '
            }

            // Everything else
            else
            {
                while (pos < length &&
                       !std::isspace(static_cast<unsigned char>(line[pos])) &&
                       line[pos] != ',' && line[pos] != ';' && line[pos] != ':' &&
                       line[pos] != '(' && line[pos] != ')' && line[pos] != '[' &&
                       line[pos] != ']' && line[pos] != '{' && line[pos] != '}' &&
                       line[pos] != '"' && line[pos] != '\'' &&
                       line[pos] != '+' && line[pos] != '-' && line[pos] != '*' && line[pos] != '/' && line[pos] != '%')
                    pos++;
                
                tokens.addToken(
                    Tokens::Type::Token,
                    context.stringPool->GetString(line.substr(startPos, pos - startPos)) ,
                    lineNumber, startPos + 1,
                    file, tokenSpaceCount
                );
            }
        }

        tokens.addToken(
            Tokens::Type::EOL,
            context.stringPool->empty(),
            lineNumber, length + 1,
            file, spaceCountLine
        );
    }

    tokens.addToken(
        Tokens::Type::EOFile,
        context.stringPool->empty(),
        lineNumber + 1, 0,
        file, 0
    );
}

Assembler::Tokens Assembler::Tokenizer::Tokenize(const Context& context, const std::vector<std::string>& filenames)
{
    Tokens tokens;

    for (const std::string& filename : filenames)
    {
        Tokenize(context, filename.c_str(), tokens);
    }

    return tokens;
}

void Assembler::Tokens::Print(std::ostream* out) const
{
    (*out) << "Tokens: " << '\n';
    for (const Token& token : tokens)
        (*out) << "  " << token.what() << '\n';
    out->flush();
}

std::string Assembler::Tokens::Token::what() const noexcept
{
    const char* type_str;

    switch (type)
    {
        case Assembler::Tokens::Type::Token:       type_str = "Token      "; break;
        case Assembler::Tokens::Type::String:      type_str = "String     "; break;
        case Assembler::Tokens::Type::Character:   type_str = "Character  "; break;
        case Assembler::Tokens::Type::Operator:    type_str = "Operator   "; break;
        case Assembler::Tokens::Type::Comma:       type_str = "Comma      "; break;
        case Assembler::Tokens::Type::Punctuation: type_str = "Punctuation"; break;
        case Assembler::Tokens::Type::Bracket:     type_str = "Bracket    "; break;
        case Assembler::Tokens::Type::EOL:         type_str = "End of line"; break;
        case Assembler::Tokens::Type::EOFile:      type_str = "End of file"; break;
        default:                                   type_str = "    Unknown"; break;
    }

    std::string result = std::string("Token (Type=") + type_str + ")";

    result +=
        " In file '" + file.str() + "'" +
        " in line " + std::to_string(line) +
        " at column " + std::to_string(column) +
        " with " + std::to_string(spaceCount) + ((spaceCount == 1) ? " space" : " spaces");

    switch (type)
    {
        case Type::Comma:
        case Type::EOL:
        case Type::EOFile:
            break;

        case Type::Character:
        {
            unsigned char c = static_cast<unsigned char>(value[0]);
            result += std::to_string(c);
            break;
        }

        case Type::Token:
        case Type::String:
        case Type::Bracket:
        case Type::Punctuation:
        default:
            result += std::string(" '") + value.c_str() + "'";
            break;
    }

    return result;
}
