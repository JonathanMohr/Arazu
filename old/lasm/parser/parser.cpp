#include "parser.hpp"

Assembler::Parser::Parser(const Context& _context, Architecture _arch, BitMode _bits)
    : context(_context), arch(_arch), bits(_bits)
{

}

Assembler::AsmProg Assembler::Parser::Parse(const Tokens& tokens)
{
    currentBitMode = bits;
    sectionMap.clear();

    AsmProg asmProg;
    asmProg.architecture = arch;
    asmProg.bits = bits;

    std::vector<AsmProg::SectionEntry> implicitSectionEntries;
    currentEntryVector = &implicitSectionEntries;
    currentSectionName = context.stringPool->GetString(".text"); // TODO: HARDCODED, UGLY

    handleLabels(asmProg, tokens);

    for (uint64_t i = 0; i < tokens.size(); i++)
    {
        const Tokens::Token& token = tokens[i];
        if (token.type == Tokens::Type::EOL || token.type == Tokens::Type::EOFile)
            continue;

        bool requireIdentifier = true;

        auto [isDirective, canContinueDirective, directiveSectionEntry] = parseDirective(asmProg, tokens, i);
        if (isDirective)
        {
            if (!canContinueDirective && i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected end of line after directive", tokens[i].line, tokens[i].column, tokens[i].file.c_str());
            
            if (directiveSectionEntry.has_value())
                currentEntryVector->push_back(directiveSectionEntry.value());

            if (!canContinueDirective) continue;
            requireIdentifier = false;
        }

        auto [isSymbol, canContinueSymbol, symbolSectionEntry] = parseSymbol(asmProg, tokens, i);
        if (isSymbol)
        {
            if (!canContinueSymbol && i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected end of line after symbol", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            if (symbolSectionEntry.has_value())
                currentEntryVector->push_back(symbolSectionEntry.value());

            if (!canContinueSymbol) continue;
            requireIdentifier = false;
        }

        auto [isPseudo, canContinuePseudo, pseudoSectionEntry] = parsePseudo(asmProg, tokens, i);
        if (isPseudo)
        {
            if (!canContinuePseudo && i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected end of line after pseudo instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            if (pseudoSectionEntry.has_value())
                currentEntryVector->push_back(pseudoSectionEntry.value());

            if (!canContinuePseudo) continue;
            requireIdentifier = false;
        }

        auto [isInstruction, canContinueInstruction, instructionSectionEntry] = parseInstruction(asmProg, tokens, i);
        if (isInstruction)
        {
            if (!canContinueInstruction && i < tokens.size() && tokens[i].type != Tokens::Type::EOL)
                throw Exception::SyntaxError("Expected end of line after instruction", tokens[i].line, tokens[i].column, tokens[i].file.c_str());

            if (instructionSectionEntry.has_value())
                currentEntryVector->push_back(instructionSectionEntry.value());

            if (!canContinueInstruction) continue;
            requireIdentifier = false;
        }

        if (requireIdentifier)
            throw Exception::SyntaxError("Invalid identifier", token.line, token.column, token.file.c_str());
    }

    for (const auto& [name, info] : globals)
    {
        if (!info.defined) // TODO: Maybe be stricter and throw an error
            context.warningManager->add(Warning::UnusedWarning("Symbol '" + name + "' was declared global but never defined", info.line, info.column, info.file.c_str()));
    }

    if (!implicitSectionEntries.empty())
    {
        auto it = sectionMap.find(".text");
        if (it != sectionMap.end())
        {
            AsmProg::Section* section = &asmProg.sections[it->second];
            section->entries.insert(section->entries.begin(), implicitSectionEntries.begin(), implicitSectionEntries.end());
        }
        else
        {
            AsmProg::Section section;

            // TODO
            section.name = context.stringPool->GetString(".text");
            section.align = 16;
            section.entries = std::move(implicitSectionEntries);

            section.isInitialized = true;
            section.isAllocatable = true;
            section.isExecutable = true;

            asmProg.sections.push_back(std::move(section));
            sectionMap[section.name.c_str()] = (uint64_t)asmProg.sections.size() - 1;
        }
    }

    return asmProg;
}

uint64_t Assembler::Parser::addLabel(AsmProg& asmProg, const AsmProg::Label& label)
{
    uint64_t index = asmProg.symbols.size();

    if (asmProg.symbolMap.find(label.name.str()) != asmProg.symbolMap.end())
        throw Exception::SemanticError("Symbol already defined", label.line, label.column, label.file.c_str());

    asmProg.symbols.push_back(label);
    asmProg.labelMap[label.name.str()] = index;
    asmProg.symbolMap[label.name.str()] = std::make_pair(true, index);
    return index;
}

uint64_t Assembler::Parser::addConstant(AsmProg& asmProg, const AsmProg::Constant& constant)
{
    uint64_t index = asmProg.symbols.size();

    if (asmProg.symbolMap.find(constant.name.str()) != asmProg.symbolMap.end())
        throw Exception::SemanticError("Symbol already defined", constant.line, constant.column, constant.file.c_str());

    asmProg.symbols.push_back(constant);
    asmProg.constantMap[constant.name.str()] = index;
    asmProg.symbolMap[constant.name.str()] = std::make_pair(false, index);
    return index;
}

Assembler::AsmProg::Section* Assembler::Parser::getSection(AsmProg& asmProg, const StringPool::String& name)
{
    auto it = sectionMap.find(name.c_str());
    if (it != sectionMap.end())
    {
        uint64_t index = it->second;
        return &asmProg.sections[index];
    }

    // Create new section
    asmProg.sections.emplace_back();
    AsmProg::Section& section = asmProg.sections.back();
    section.name = name;

    sectionMap[name.c_str()] = (uint64_t)asmProg.sections.size() - 1;

    return &section;
}

#include "x86/parser.hpp"

std::unique_ptr<Assembler::Parser> Assembler::Parser::Get(const Context& c, Architecture a, BitMode b)
{
    switch (a)
    {
        case Architecture::x86: return std::make_unique<x86::Parser>(c, a, b);
        default: return nullptr;
    }
}
