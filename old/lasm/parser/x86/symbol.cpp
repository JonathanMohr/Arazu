#include "parser.hpp"

#include <util/string.hpp>
#include "../evaluate.hpp"

#include "data.hpp"

Assembler::AsmProg::Immediate::Operand Assembler::x86::Parser::getImmediateOperand(const Tokens::Token& token)
{
    if (token.type == Tokens::Type::Operator || token.type == Tokens::Type::Bracket)
    {
        // TODO: Check
        AsmProg::Operator op;
        op.op = token.value[0];
        return op;
    }
    else if (token.type == Tokens::Type::Character)
    {
        AsmProg::Integer integer;
        // TODO: Check
        integer.value = static_cast<uint64_t>(static_cast<unsigned char>(token.value[0]));
        return integer;
    }
    else if (token.type == Tokens::Type::Token && (token.value == "$" ||token.value == "$$"))
    {
        AsmProg::Position position;
        position.sectionPos = (token.value != "$");
        return position;
    }
    else if (std::isdigit(static_cast<unsigned char>(token.value[0])) != 0)
    {
        AsmProg::Integer integer;
        // TODO: Make better
        integer.value = evalInteger(token.value, 8, token.line, token.column, token.file.c_str());
        return integer;
    }
    else // TODO: String?
    {
        AsmProg::String string;

        if (token.value.size() > 0 && token.value[0] == '.')
            string.value = lastMainLabel + token.value;
        else
            string.value = token.value;

        return string;
    }
}

std::tuple<bool, bool, std::optional<Assembler::AsmProg::SectionEntry>> Assembler::x86::Parser::parseSymbol(AsmProg& asmProg, const Tokens& tokens, uint64_t& i)
{
    const Tokens::Token& token = tokens[i];
    // TODO: const std::string lowerValue = toLower(token.value);

    // Constants (equ)
    if (i + 1 < tokens.size() && tokens[i + 1].type == Tokens::Type::Token && tokens[i + 1].value == "equ")
    {
        AsmProg::Constant constant;
        constant.line = token.line;
        constant.column = token.column;
        constant.file = token.file;

        constant.name = token.value;
        constant.section = currentSectionName;
        constant.isGlobal = globals.find(token.value.str()) != globals.end();
        if (constant.isGlobal) globals.at(token.value.str()).defined = true;

        i += 2;

        while (i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
        {
            if (
                tokens[i].type != Tokens::Type::Token &&
                tokens[i].type != Tokens::Type::Operator &&
                tokens[i].type != Tokens::Type::Character &&
                tokens[i].type != Tokens::Type::Bracket
            ) throw Exception::SyntaxError("Expected definition after 'equ'", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            AsmProg::Immediate::Operand operand = getImmediateOperand(tokens[i]);
            constant.expression.operands.push_back(operand);
            i++;
        }

        AsmProg::SectionEntry::Constant sectionEntryConstant(addConstant(asmProg, constant));
        return std::make_tuple(true, false, sectionEntryConstant);
    }

    // Labels
    if (token.type == Tokens::Type::Token && i + 1 < tokens.size() && (
        (tokens[i + 1].type == Tokens::Type::Punctuation && tokens[i + 1].value == ":" /* TODO: add segment:offset */)
     || (isDataDefinition(tokens[i + 1]))
    )) {
        AsmProg::Label label;
        label.line = token.line;
        label.column = token.column;
        label.file = token.file;

        label.section = currentSectionName;
        label.isGlobal = globals.find(token.value.str()) != globals.end();
        if (label.isGlobal) globals.at(token.value.str()).defined = true;
        label.isExtern = false;

        if (token.value.size() > 0 && token.value[0] == '.')
            label.name = lastMainLabel + token.value;
        else
        {
            label.name = token.value;
            lastMainLabel = label.name;
        }

        i++;
        if (tokens[i].type == Tokens::Type::Punctuation) i++;

        AsmProg::SectionEntry::Label sectionEntryLabel(addLabel(asmProg, label));
        // TODO: Could continue after line
        return std::make_tuple(true, true, sectionEntryLabel);
    }

    return std::make_tuple(false, false, std::nullopt);
}
