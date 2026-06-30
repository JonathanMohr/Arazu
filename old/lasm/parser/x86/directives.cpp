#include "parser.hpp"

#include <algorithm>
#include <array>
#include <string_view>

#include <util/string.hpp>
#include "../evaluate.hpp"

void Assembler::x86::Parser::handleLabels(AsmProg& asmProg, const Tokens& tokens)
{
    (void)asmProg;

    for (uint64_t i = 0; i < tokens.size(); i++)
    {
        const Tokens::Token& token = tokens[i];
        const std::string lowerValue = toLower(token.value);

        if (token.type == Tokens::Type::Bracket && token.value == "[")
        {
            if (i + 1 >= tokens.size())
                throw Exception::SyntaxError("Missing directive after '['", token.line, token.column, token.file.c_str());
        }

        static constexpr std::array<std::string_view, 1> directives = {
            "global"
        };

        if (
            (token.type == Tokens::Type::Bracket && token.value == "[" && std::find(directives.begin(), directives.end(), toLower(tokens[i + 1].value)) != directives.end())
         || std::find(directives.begin(), directives.end(), lowerValue) != directives.end()
        ) {
            if (token.type == Tokens::Type::Bracket) i++;

            const Tokens::Token& directive = tokens[i];
            const std::string lowerDirective = toLower(directive.value);

            if (i + 1 >= tokens.size() || tokens[i + 1].type != Tokens::Type::Token) throw Exception::SyntaxError("Missing label after directive", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
            i++;

            std::string rawName = tokens[i].value.str();
            while (i + 1 < tokens.size() && tokens[i + 1].type == Tokens::Type::Token && tokens[i + 1].spaceCount == 0)
            {
                rawName += tokens[i + 1].value.str();
                i++;
            }

            i++;

            if (lowerDirective == "global")
            {
                if (globals.find(rawName) != globals.end())
                    context.warningManager->add(Warning::SemanticWarning("Label is already declared global", directive.line, directive.column, directive.file.c_str()));

                globals.emplace(
                    rawName,
                    GlobalInformation(directive.line, directive.column, directive.file)
                );
            }
            else
                throw Exception::InternalError("Invalid directive", directive.line, directive.column, directive.file.c_str());

            // TODO: Check overflow

            if (token.type == Tokens::Type::Bracket)
            {
                if (tokens[i].type != Tokens::Type::Bracket || tokens[i].value != "]")
                    throw Exception::SyntaxError("Missing Closing Bracket", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
                i++;
            }

            if (i >= tokens.size() || tokens[i].type != Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected end of line after directive", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
        }

        while (i < tokens.size() && tokens[i].type != Tokens::Type::EOL) i++;
    }
}

std::tuple<bool, bool, std::optional<Assembler::AsmProg::SectionEntry>> Assembler::x86::Parser::parseDirective(AsmProg& asmProg, const Tokens& tokens, uint64_t& i)
{
    const Tokens::Token& token = tokens[i];
    const std::string lowerValue = toLower(token.value);

    static constexpr std::array<std::string_view, 7> directives = {
        "global",
        "extern", "section", "segment", "bits", "org", "align"
    };

    if (token.type == Tokens::Type::Bracket && token.value == "[")
    {
        if (i + 1 >= tokens.size())
            throw Exception::SyntaxError("Missing directive after '['", token.line, token.column, token.file.c_str());
    }

    // TODO: Don't mistake labels for directives
    if (
        (token.type == Tokens::Type::Bracket && token.value == "[" && std::find(directives.begin(), directives.end(), toLower(tokens[i + 1].value)) != directives.end())
     || std::find(directives.begin(), directives.end(), lowerValue) != directives.end()
    ) {
        if (token.type == Tokens::Type::Bracket) i++;

        std::optional<Assembler::AsmProg::SectionEntry> potentialEntry = std::nullopt;

        const Tokens::Token& directive = tokens[i];
        const std::string lowerDirective = toLower(directive.value);

        if (lowerDirective == "global")
        {
            // Already handled
            while (i < tokens.size() && tokens[i].type != Tokens::Type::EOL) i++;
        }
        else if (lowerDirective == "extern")
        {
            // TODO: I don't actually think, that this makes sense
            if (i + 1 >= tokens.size() || tokens[i + 1].type != Tokens::Type::Token) throw Exception::SyntaxError("Missing name after directive", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
            i++;

            const Tokens::Token& startToken = tokens[i];

            std::string rawName = tokens[i].value.str();
            while (i + 1 < tokens.size() && tokens[i + 1].type == Tokens::Type::Token && tokens[i + 1].spaceCount == 0)
            {
                rawName += tokens[i + 1].value.str();
                i++;
            }

            i++;

            AsmProg::Label label;
            
            label.name = context.stringPool->GetString(rawName);
            label.section = currentSectionName;

            label.line = startToken.line;
            label.column = startToken.column;
            label.file = startToken.file;

            label.isGlobal = isLabelGlobal(label.name);
            label.isExtern = true;

            AsmProg::SectionEntry::Label sectionEntryLabel(addLabel(asmProg, label));
            potentialEntry = sectionEntryLabel;
        }
        else if (lowerDirective == "section" || lowerDirective == "segment")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1].type != Tokens::Type::Token) throw Exception::SyntaxError("Missing section name after directive", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
            i++;

            std::string rawName = tokens[i].value.str();
            while (i + 1 < tokens.size() && tokens[i + 1].type == Tokens::Type::Token && tokens[i + 1].spaceCount == 0)
            {
                rawName += tokens[i + 1].value.str();
                i++;
            }

            StringPool::String name = context.stringPool->GetString(rawName);
            AsmProg::Section* currentSection = getSection(asmProg, name);
            currentEntryVector = &currentSection->entries;
            currentSectionName = name;

            if (name == ".text")
            {
                currentSection->align = 16;

                currentSection->isInitialized = true;
                currentSection->isAllocatable = true;
                currentSection->isExecutable = true;
            }
            else if (name == ".data")
            {
                currentSection->align = 4;

                currentSection->isInitialized = true;
                currentSection->isAllocatable = true;
                currentSection->isWritable = true;
            }
            else if (name == ".rodata")
            {
                currentSection->align = 4;

                currentSection->isInitialized = true;
                currentSection->isAllocatable = true;
            }
            else if (name == ".bss")
            {
                currentSection->align = 4;

                currentSection->isAllocatable = true;
                currentSection->isWritable = true;
            }
            else
            {
                currentSection->align = 4;

                // TODO
                currentSection->isAllocatable = true;
            }

            i++;

            while (i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
            {
                const std::string extraArgument = toLower(tokens[i].value);

                if (extraArgument.find("align") == 0)
                {
                    uint64_t pos = tokens[i].value.str().find('=');
                    if (pos != 5)
                        throw Exception::SyntaxError("Section attribute 'align' requires a value", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

                    const std::string alignValue = extraArgument.substr(6);
                    if (alignValue.empty()) throw Exception::SyntaxError("Section attribute 'align' requires a value", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

                    uint64_t align = evalInteger(context.stringPool->GetString(alignValue), 8, tokens[i].line, tokens[i].column, tokens[i].file.c_str());
                    currentSection->align = align;

                    // TODO: Validate that it's a power of two
                }
                else
                    throw Exception::SyntaxError("Invalid section attribute: '" + extraArgument + "'", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

                i++;
            }
        }
        else if (lowerDirective == "bits")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1].type != Tokens::Type::Token)
                throw Exception::SyntaxError("Missing bits after directive", directive.line, directive.column, directive.file.c_str());
            
            i++;
            const StringPool::String& bit_str = tokens[i].value;

            if      (bit_str == "16") currentBitMode = BitMode::Bits16;
            else if (bit_str == "32") currentBitMode = BitMode::Bits32;
            else if (bit_str == "64") currentBitMode = BitMode::Bits64;
            else
                throw Exception::SyntaxError("Invalid bits", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            i++;
        }
        else if (lowerDirective == "org")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1].type != Tokens::Type::Token)
                throw Exception::SyntaxError("Missing origin after directive", directive.line, directive.column, directive.file.c_str());
            i++;

            SetOrigin(asmProg, tokens[i].value.c_str());

            i++;
        }
        else if (lowerDirective == "align")
        {
            // TODO
            throw Exception::InternalError("align not implemented yet", directive.line, directive.column, directive.file.c_str());
        }
        else
            throw Exception::InternalError("Invalid directive", directive.line, directive.column, directive.file.c_str());

        // TODO: Check overflow

        if (token.type == Tokens::Type::Bracket)
        {
            if (tokens[i].type != Tokens::Type::Bracket || tokens[i].value != "]")
                throw Exception::SyntaxError("Missing Closing Bracket", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
            i++;
        }

        return std::make_tuple(true, false, potentialEntry);
    }

    return std::make_tuple(false, false, std::nullopt);
}
