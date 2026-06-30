#include "parser.hpp"

#include <util/string.hpp>

#include "data.hpp"

#include "../../instruction/data.hpp"

std::tuple<bool, bool, std::optional<Assembler::AsmProg::SectionEntry>> Assembler::x86::Parser::parsePseudo(AsmProg& asmProg, const Tokens& tokens, uint64_t& i)
{
    (void)asmProg;

    const Tokens::Token& token = tokens[i];
    const std::string lowerValue = toLower(token.value);

    // times
    if (token.type == Tokens::Type::Token && lowerValue == "times")
    {
        throw Exception::InternalError("times not implemented yet", token.line, token.column, token.file.c_str());
    }

    // Data
    if (isDataDefinition(token))
    {
        bool isReserved = false;
        char cSize = '\0';

        if (lowerValue.compare(0, 3, "res") == 0)
        {
            isReserved = true;
            cSize = lowerValue[3];
        }
        else if (lowerValue[0] == 'd')
        {
            isReserved = false;
            cSize = lowerValue[1];
        }
        else
            throw Exception::InternalError("Invalid data pseudo instruction", token.line, token.column, token.file.c_str());

        uint64_t dataSize;

        switch (cSize)
        {
            case 'b': dataSize = 1; break;
            case 'w': dataSize = 2; break;
            case 'd': dataSize = 4; break;
            case 'q': dataSize = 8; break;
            case 't': dataSize = 10; break;
            case 'o': dataSize = 16; break;
            case 'y': dataSize = 32; break;
            case 'z': dataSize = 64; break;
            default:
                throw Exception::InternalError("Invalid size suffix", token.line, token.column, token.file.c_str());
        }

        i++;

        if (isReserved)
        {
            if (i >= tokens.size())
                throw Exception::SyntaxError("Missing value after data definition", token.line, token.column, token.file.c_str());

            AsmProg::Immediate value;

            if (tokens[i].type == Tokens::Type::Token
             || tokens[i].type == Tokens::Type::Operator
             || tokens[i].type == Tokens::Type::Character
             || tokens[i].type == Tokens::Type::Bracket)
            {
                while (i < tokens.size() &&
                    !(tokens[i].type == Tokens::Type::Comma || tokens[i].type == Tokens::Type::EOL))
                {
                    value.operands.push_back(getImmediateOperand(tokens[i]));
                    i++;
                }
            }
            else
                throw Exception::SyntaxError("Expected value after data pseudo instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            if (i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected end of line after value", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            std::shared_ptr<Assembler::Instruction> dataEntry = std::make_shared<Instructions::Reserved>(dataSize, value, context.stringPool->empty(), token.line, token.column, token.file);
            return std::make_tuple(true, false, std::move(dataEntry));
        }
        else
        {
            std::vector<AsmProg::Immediate> values;

            while (i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
            {
                if (tokens[i].type == Tokens::Type::Token
                 || tokens[i].type == Tokens::Type::Operator
                 || tokens[i].type == Tokens::Type::Character
                 || tokens[i].type == Tokens::Type::Bracket)
                {
                    AsmProg::Immediate value;

                    while (i < tokens.size() &&
                        !(tokens[i].type == Tokens::Type::Comma || tokens[i].type == Tokens::Type::EOL))
                    {
                        value.operands.push_back(getImmediateOperand(tokens[i]));
                        i++;
                    }

                    values.push_back(std::move(value));
                }
                else if (tokens[i].type == Tokens::Type::String)
                {
                    const StringPool::String& value = tokens[i].value;
                    uint64_t len = value.size();

                    for (uint64_t pos = 0; pos < len; pos += dataSize)
                    {
                        uint64_t combined = 0;

                        for (uint64_t offset = 0; offset < dataSize; offset++)
                        {
                            if (pos + offset < len)
                                combined |= static_cast<uint64_t>(static_cast<unsigned char>(value[pos + offset])) << (8 * offset);
                            else // TODO: Add Padding 0s
                                combined |= 0ULL << (8 * offset);
                        }

                        AsmProg::Immediate immediate;

                        AsmProg::Integer integer;
                        integer.value = combined;
                        immediate.operands.push_back(integer);

                        values.push_back(std::move(immediate));
                    }

                    i++;
                }
                else
                    throw Exception::SyntaxError("Expected value after data pseudo instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

                if (i >= tokens.size()) break;

                if (tokens[i].type == Tokens::Type::Comma)
                {
                    i++;
                    if (i >= tokens.size() || tokens[i].type == Tokens::Type::EOL)
                        throw Exception::SyntaxError("Expected value after comma", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
                }
                else if (tokens[i].type != Tokens::Type::EOL)
                    throw Exception::SyntaxError("Expected comma or end of line after value", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
            }

            std::shared_ptr<Assembler::Instruction> dataEntry = std::make_shared<Instructions::Data>(dataSize, values, token.line, token.column, token.file);
            return std::make_tuple(true, false, std::move(dataEntry));
        }
    }

    return std::make_tuple(false, false, std::nullopt);
}
