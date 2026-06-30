#include "elf.hpp"

#include <cstring>
#include <limits>
#include <queue>
#include "header.hpp"

void Assembler::ELFWriter::Write(const Object& object, std::ostream* _output)
{
    position = 0;
    output = _output;
    useELF32 = (getBits(object) == BitMode::Bits16 || getBits(object) == BitMode::Bits32);

    constexpr uint64_t alignTo = 16;

    std::vector<Object::Section> sections = getSections(object);
    std::vector<Object::Symbol> symbols;
    symbols.reserve(getSymbols(object).size());

    for (const Object::Symbol& symbol : getSymbols(object))
    {
        if (symbol.isGlobal) continue;
        symbols.push_back(symbol);
    }

    for (const Object::Symbol& symbol : getSymbols(object))
    {
        if (!symbol.isGlobal) continue;
        symbols.push_back(symbol);
    }

    // null, [sections], .shstrtab, .symtab, .strtab, [relocation sections]
    std::vector<uint8_t> shstrtabBuffer = {0};
    std::vector<uint8_t> symtabBuffer;
    uint64_t localSymbolsCount = 0;
    uint64_t symbolCount = 0;

    std::vector<uint8_t> strtabBuffer = {0};

    std::unordered_map<std::string, uint64_t> symbolIndices;

    auto insertSymbol32 = [&](const ELF::SymbolEntry32& entry, const StringPool::String* name)
    {
        if (ELF::SymbolInfo::GetBind(entry.info) == ELF::SymbolInfo::BIND_LOCAL) localSymbolsCount++;

        if (name) symbolIndices[name->str()] = symbolCount;
        symbolCount++;

        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&entry);
        symtabBuffer.insert(symtabBuffer.end(), bytes, bytes + sizeof(entry));
    };
    auto insertSymbol64 = [&](const ELF::SymbolEntry64& entry, const StringPool::String* name)
    {
        if (ELF::SymbolInfo::GetBind(entry.info) == ELF::SymbolInfo::BIND_LOCAL) localSymbolsCount++;
        
        if (name) symbolIndices[name->str()] = symbolCount;
        symbolCount++;

        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&entry);
        symtabBuffer.insert(symtabBuffer.end(), bytes, bytes + sizeof(entry));
    };

    if (useELF32)
    {
        ELF::SymbolEntry32 nullEntry;
        nullEntry.indexInStringTable = 0;
        nullEntry.value = 0;
        nullEntry.size = 0;
        nullEntry.info = 0;
        nullEntry.other = 0;
        nullEntry.sectionIndex = 0;
        insertSymbol32(nullEntry, nullptr);
    }
    else
    {
        ELF::SymbolEntry64 nullEntry;
        nullEntry.indexInStringTable = 0;
        nullEntry.value = 0;
        nullEntry.size = 0;
        nullEntry.info = 0;
        nullEntry.other = 0;
        nullEntry.sectionIndex = 0;
        insertSymbol64(nullEntry, nullptr);
    }


    if (strtabBuffer.size() > std::numeric_limits<uint32_t>::max())
        throw Exception::OverflowError(".strtab too big", 0, 0);

    uint32_t filenameOffset = static_cast<uint32_t>(strtabBuffer.size());
    strtabBuffer.insert(strtabBuffer.end(), context.filename.begin(), context.filename.end());
    strtabBuffer.push_back(0);

    if (useELF32)
    {
        ELF::SymbolEntry32 entry;
        entry.indexInStringTable = filenameOffset;
        entry.value = 0;
        entry.size = 0;
        entry.info = ELF::SymbolInfo::GetInfo(ELF::SymbolInfo::BIND_LOCAL, ELF::SymbolInfo::TYPE_FILE);
        entry.other = 0;
        entry.sectionIndex = ELF::SectionOffsets::ABSOLUTE;
        insertSymbol32(entry, &context.filename);
    }
    else
    {
        ELF::SymbolEntry64 entry;
        entry.indexInStringTable = filenameOffset;
        entry.value = 0;
        entry.size = 0;
        entry.info = ELF::SymbolInfo::GetInfo(ELF::SymbolInfo::BIND_LOCAL, ELF::SymbolInfo::TYPE_FILE);
        entry.other = 0;
        entry.sectionIndex = ELF::SectionOffsets::ABSOLUTE;
        insertSymbol64(entry, &context.filename);
    }

    uint64_t sectionCount = sections.size() + 1;

    const uint64_t shstrtabIndex = sectionCount;
    const uint64_t symtabIndex = shstrtabIndex + 1;
    const uint64_t strtabIndex = shstrtabIndex + 2;
    sectionCount += 3;

    for (const Object::Section& section : sections)
    {
        if (!section.relocations.empty()) sectionCount++;
    }

    uint16_t instructionSet;

    // Header
    uint64_t sectionHeaderTablePosition;
    uint64_t sectionContentStart;
    if (useELF32)
    {
        ELF::Header32 header;
        header.bitness = ELF::Bitness::BITS32;
        header.endianness = ELF::Endianness::LSB2;
        header.headerVersion = ELF::Version::CURRENT;
        header.abi = ELF::ABI::NONE;

        header.type = ELF::Type::RELOCATABLE;

        switch (getArchitecture(object))
        {
            case Architecture::x86:
                header.instructionSet = ELF::InstructionSet::I386;
                break;

            case Architecture::ARM:
                header.instructionSet = ELF::InstructionSet::ARM;
                break;

            case Architecture::RISC_V:
                header.instructionSet = ELF::InstructionSet::RISCV;
                break;

            default:
                throw Exception::InternalError("Invalid architecture", 0, 0);
        }

        instructionSet = header.instructionSet;

        header.version = ELF::Version::CURRENT;

        header.flags = 0; // TODO

        header.headerSize = sizeof(ELF::Header32);

        header.programEntryPosition = 0;
        header.programHeaderTablePosition = 0;
        header.sectionHeaderTablePosition = ((header.headerSize + alignTo - 1) / alignTo) * alignTo;

        //TODO: header.programHeaderTableEntrySize = sizeof(ELF::ProgramSegmentHeader32);
        header.programHeaderTableEntrySize = 0;
        header.programHeaderTableEntryCount = 0;

        header.sectionHeaderTableEntrySize = sizeof(ELF::SectionHeader32);
        
        if (sectionCount > static_cast<uint64_t>(std::numeric_limits<uint16_t>::max()))
            throw Exception::OverflowError("Too many sections for ELF32", 0, 0);
        else
            header.sectionHeaderTableEntryCount = static_cast<uint16_t>(sectionCount);

        if (shstrtabIndex > static_cast<uint64_t>(std::numeric_limits<uint16_t>::max()))
            throw Exception::OverflowError("Too many sections for ELF32", 0, 0);
        else
            header.sectionNamesIndex = static_cast<uint16_t>(shstrtabIndex);

        sectionHeaderTablePosition = header.sectionHeaderTablePosition;
        sectionContentStart = sectionHeaderTablePosition + (uint64_t)header.sectionHeaderTableEntrySize * header.sectionHeaderTableEntryCount;

        WriteAll(&header, sizeof(header));
    }
    else
    {
        ELF::Header64 header;
        header.bitness = ELF::Bitness::BITS64;
        header.endianness = ELF::Endianness::LSB2;
        header.headerVersion = ELF::Version::CURRENT;
        header.abi = ELF::ABI::NONE;

        header.type = ELF::Type::RELOCATABLE;

        switch (getArchitecture(object))
        {
            case Architecture::x86:
                header.instructionSet = ELF::InstructionSet::X86_64;
                break;

            case Architecture::ARM:
                header.instructionSet = ELF::InstructionSet::AARCH64;
                break;

            case Architecture::RISC_V:
                header.instructionSet = ELF::InstructionSet::RISCV;
                break;

            default:
                throw Exception::InternalError("Invalid architecture", 0, 0);
        }

        instructionSet = header.instructionSet;

        header.version = ELF::Version::CURRENT;

        header.flags = 0; // TODO

        header.headerSize = sizeof(ELF::Header64);

        header.programEntryPosition = 0;
        header.programHeaderTablePosition = 0;
        header.sectionHeaderTablePosition = ((header.headerSize + alignTo - 1) / alignTo) * alignTo;

        //TODO: header.programHeaderTableEntrySize = sizeof(ELF::ProgramSegmentHeader64);
        header.programHeaderTableEntrySize = 0;
        header.programHeaderTableEntryCount = 0;

        header.sectionHeaderTableEntrySize = sizeof(ELF::SectionHeader64);
        
        if (sectionCount > static_cast<uint64_t>(std::numeric_limits<uint16_t>::max()))
            throw Exception::OverflowError("Too many sections for ELF64", 0, 0);
        else
            header.sectionHeaderTableEntryCount = static_cast<uint16_t>(sectionCount);

        if (shstrtabIndex > static_cast<uint64_t>(std::numeric_limits<uint16_t>::max()))
            throw Exception::OverflowError("Too many sections for ELF64", 0, 0);
        else
            header.sectionNamesIndex = static_cast<uint16_t>(shstrtabIndex);

        sectionHeaderTablePosition = header.sectionHeaderTablePosition;
        sectionContentStart = sectionHeaderTablePosition + (uint64_t)header.sectionHeaderTableEntrySize * header.sectionHeaderTableEntryCount;

        WriteAll(&header, sizeof(header));
    }

    WritePadding(sectionHeaderTablePosition);

    WriteNullSectionHeader();
    uint64_t sectionIndex = 1;

    std::unordered_map<std::string, uint64_t> sectionIndices;

    uint64_t sectionContentOffset = ((sectionContentStart + alignTo - 1) / alignTo) * alignTo;
    for (const Object::Section& section : sections)
    {
        if (shstrtabBuffer.size() > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError(".shstrtab too big", 0, 0);

        uint32_t nameOffset = static_cast<uint32_t>(shstrtabBuffer.size());
        shstrtabBuffer.insert(shstrtabBuffer.end(), section.name.begin(), section.name.end());
        shstrtabBuffer.push_back(0);

        if (useELF32)
        {
            ELF::SectionHeader32 header;
            header.offsetInSectionNameStringTable = nameOffset;
            
            switch (section.type)
            {
                case Object::Section::Type::Initialized:
                    header.type = ELF::SectionTypes::PROGBITS;
                    break;
                
                case Object::Section::Type::Uninitialized:
                    header.type = ELF::SectionTypes::NOBITS;
                    break;

                default:
                    throw Exception::InternalError("Invalid section type", 0, 0);
            }

            header.flags = 0;
            if (section.flags & Object::Section::FlagsAllocatable)
                header.flags |= ELF::SectionFlags::ALLOC;
            if (section.flags & Object::Section::FlagsExecutable)
                header.flags |= ELF::SectionFlags::EXECINSTR;
            if (section.flags & Object::Section::FlagsWritable)
                header.flags |= ELF::SectionFlags::WRITE;

            if (sectionContentOffset > std::numeric_limits<uint32_t>::max())
                throw Exception::OverflowError("Section offset in file too big for ELF32", 0, 0);
            header.offset = static_cast<uint32_t>(sectionContentOffset);

            if (section.size > std::numeric_limits<uint32_t>::max())
                throw Exception::OverflowError("Section size too big for ELF32", 0, 0);
            header.sectionSize = static_cast<uint32_t>(section.size);

            if (section.align > std::numeric_limits<uint32_t>::max())
                throw Exception::OverflowError("Section alignment too big for ELF32", 0, 0);
            header.addressAlignment = static_cast<uint32_t>(section.align);

            header.entrySize = 0;

            header.virtualAddress = 0; // TODO
            header.linkIndex = 0; // TODO
            header.info = 0;      // TODO

            WriteAll(&header, sizeof(header));

            ELF::SymbolEntry32 entry;
            entry.indexInStringTable = 0;
            entry.value = 0;
            entry.size = 0;
            entry.info = ELF::SymbolInfo::GetInfo(ELF::SymbolInfo::BIND_LOCAL, ELF::SymbolInfo::TYPE_SECTION);
            entry.other = 0;

            if (sectionIndex > std::numeric_limits<uint16_t>::max())
                throw Exception::OverflowError("section index too big", 0, 0);
            entry.sectionIndex = static_cast<uint16_t>(sectionIndex);

            insertSymbol32(entry, &section.name);
        }
        else
        {
            ELF::SectionHeader64 header;
            header.offsetInSectionNameStringTable = nameOffset;
            
            switch (section.type)
            {
                case Object::Section::Type::Initialized:
                    header.type = ELF::SectionTypes::PROGBITS;
                    break;
                
                case Object::Section::Type::Uninitialized:
                    header.type = ELF::SectionTypes::NOBITS;
                    break;

                default:
                    throw Exception::InternalError("Invalid section type", 0, 0);
            }

            header.flags = 0;
            if (section.flags & Object::Section::FlagsAllocatable)
                header.flags |= ELF::SectionFlags::ALLOC;
            if (section.flags & Object::Section::FlagsExecutable)
                header.flags |= ELF::SectionFlags::EXECINSTR;
            if (section.flags & Object::Section::FlagsWritable)
                header.flags |= ELF::SectionFlags::WRITE;

            header.offset = sectionContentOffset;
            header.sectionSize = section.size;
            header.addressAlignment = section.align;

            header.entrySize = 0;

            header.virtualAddress = 0; // TODO
            header.linkIndex = 0; // TODO
            header.info = 0;      // TODO

            WriteAll(&header, sizeof(header));

            ELF::SymbolEntry64 entry;
            entry.indexInStringTable = 0;
            entry.value = 0;
            entry.size = 0;
            entry.info = ELF::SymbolInfo::GetInfo(ELF::SymbolInfo::BIND_LOCAL, ELF::SymbolInfo::TYPE_SECTION);
            entry.other = 0;

            if (sectionIndex > std::numeric_limits<uint16_t>::max())
                throw Exception::OverflowError("section index too big", 0, 0);
            entry.sectionIndex = static_cast<uint16_t>(sectionIndex);

            insertSymbol64(entry, &section.name);
        }

        switch (section.type)
        {
            case Object::Section::Type::Initialized:
                sectionContentOffset += section.size;
                sectionContentOffset += (alignTo - (sectionContentOffset % alignTo)) % alignTo;
                break;

            case Object::Section::Type::Uninitialized:
                break;

            default:
                throw Exception::InternalError("Invalid section type", 0, 0);
        }

        sectionIndices[section.name.str()] = sectionIndex;
        sectionIndex++;
    }

    // SHSTRTAB, SYMTAB, STRTAB
    std::string shstrtabName = ".shstrtab";
    std::string symtabName = ".symtab";
    std::string strtabName = ".strtab";

    if (shstrtabBuffer.size() > std::numeric_limits<uint32_t>::max())
        throw Exception::OverflowError(".shstrtab too big", 0, 0);
    uint32_t shstrtabNameOffset = static_cast<uint32_t>(shstrtabBuffer.size());
    shstrtabBuffer.insert(shstrtabBuffer.end(), shstrtabName.begin(), shstrtabName.end());
    shstrtabBuffer.push_back(0);

    if (shstrtabBuffer.size() > std::numeric_limits<uint32_t>::max())
        throw Exception::OverflowError(".shstrtab too big", 0, 0);
    uint32_t symtabNameOffset = static_cast<uint32_t>(shstrtabBuffer.size());
    shstrtabBuffer.insert(shstrtabBuffer.end(), symtabName.begin(), symtabName.end());
    shstrtabBuffer.push_back(0);

    if (shstrtabBuffer.size() > std::numeric_limits<uint32_t>::max())
        throw Exception::OverflowError(".shstrtab too big", 0, 0);
    uint32_t strtabNameOffset = static_cast<uint32_t>(shstrtabBuffer.size());
    shstrtabBuffer.insert(shstrtabBuffer.end(), strtabName.begin(), strtabName.end());
    shstrtabBuffer.push_back(0);

    // RELOCATIONS
    std::queue<uint32_t> relocationShstrtabIndices;

    for (const Object::Section& section : sections)
    {
        if (!section.relocations.empty())
        {
            std::string relocSectionName;
            if (useELF32) relocSectionName = ".rel" + section.name.str();
            else          relocSectionName = ".rela" + section.name.str();

            if (shstrtabBuffer.size() > std::numeric_limits<uint32_t>::max())
                throw Exception::OverflowError(".shstrtab too big", 0, 0);
            uint32_t shstrtabOffset = static_cast<uint32_t>(shstrtabBuffer.size());
            shstrtabBuffer.insert(shstrtabBuffer.end(), relocSectionName.begin(), relocSectionName.end());
            shstrtabBuffer.push_back(0);

            relocationShstrtabIndices.push(shstrtabOffset);
        }
    }

    // SHSTRTAB
    if (useELF32)
    {
        ELF::SectionHeader32 header;
        header.offsetInSectionNameStringTable = shstrtabNameOffset;
        header.type = ELF::SectionTypes::STRTAB;
        header.flags = 0;

        if (sectionContentOffset > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Section offset in file too big for ELF32", 0, 0);
        header.offset = static_cast<uint32_t>(sectionContentOffset);

        if (shstrtabBuffer.size() > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Section size too big for ELF32", 0, 0);
        header.sectionSize = static_cast<uint32_t>(shstrtabBuffer.size());

        header.addressAlignment = 1;

        header.entrySize = 0;

        header.virtualAddress = 0;
        header.linkIndex = 0;
        header.info = 0;

        WriteAll(&header, sizeof(header));
    }
    else
    {
        ELF::SectionHeader64 header;
        header.offsetInSectionNameStringTable = shstrtabNameOffset;
        header.type = ELF::SectionTypes::STRTAB;
        header.flags = 0;

        header.offset = sectionContentOffset;
        header.sectionSize = static_cast<uint64_t>(shstrtabBuffer.size());
        header.addressAlignment = 1;

        header.entrySize = 0;

        header.virtualAddress = 0;
        header.linkIndex = 0;
        header.info = 0;

        WriteAll(&header, sizeof(header));
    }
    sectionContentOffset += shstrtabBuffer.size();
    sectionContentOffset += (alignTo - (sectionContentOffset % alignTo)) % alignTo;

    // SYMBOLS
    for (const Object::Symbol& symbol : symbols)
    {
        if (strtabBuffer.size() > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError(".strtab too big", 0, 0);

        uint32_t nameOffset = static_cast<uint32_t>(strtabBuffer.size());
        strtabBuffer.insert(strtabBuffer.end(), symbol.name.begin(), symbol.name.end());
        strtabBuffer.push_back(0);

        uint8_t symbolBind;
        if (symbol.isGlobal) symbolBind = ELF::SymbolInfo::BIND_GLOBAL;
        else                 symbolBind = ELF::SymbolInfo::BIND_LOCAL;

        // TODO
        uint8_t symbolType = ELF::SymbolInfo::TYPE_NONE;

        if (useELF32)
        {
            ELF::SymbolEntry32 entry;
            entry.indexInStringTable = nameOffset;

            if (symbol.value > std::numeric_limits<uint32_t>::max())
                throw Exception::OverflowError("value of '" + symbol.name.str() + "' too big", 0, 0);
            entry.value = static_cast<uint32_t>(symbol.value);
            entry.size = 0; // TODO: Add size

            entry.info = ELF::SymbolInfo::GetInfo(symbolBind, symbolType);
            entry.other = 0; // TODO

            if (symbol.section.has_value())
            {
                auto it = sectionIndices.find(symbol.section.value().str());
                if (it == sectionIndices.end())
                    throw Exception::InternalError("Could not find section '" + symbol.section.value().str() + "'", 0, 0);
                
                if (it->second > std::numeric_limits<uint16_t>::max())
                    throw Exception::OverflowError("Section index too big", 0, 0);
                entry.sectionIndex = static_cast<uint16_t>(it->second);
            }
            else
            {
                if (symbol.absolute)
                    entry.sectionIndex = ELF::SectionOffsets::ABSOLUTE;
                else
                    entry.sectionIndex = ELF::SectionOffsets::UNDEFINED;
            }

            insertSymbol32(entry, &symbol.name);
        }
        else
        {
            ELF::SymbolEntry64 entry;
            entry.indexInStringTable = nameOffset;

            entry.value = symbol.value;
            entry.size = 0; // TODO: Add size

            entry.info = ELF::SymbolInfo::GetInfo(symbolBind, symbolType);
            entry.other = 0; // TODO

            if (symbol.section.has_value())
            {
                auto it = sectionIndices.find(symbol.section.value().str());
                if (it == sectionIndices.end())
                    throw Exception::InternalError("Could not find section '" + symbol.section.value().str() + "'", 0, 0);
                
                if (it->second > std::numeric_limits<uint16_t>::max())
                    throw Exception::OverflowError("Section index too big", 0, 0);
                entry.sectionIndex = static_cast<uint16_t>(it->second);
            }
            else
            {
                if (symbol.absolute)
                    entry.sectionIndex = ELF::SectionOffsets::ABSOLUTE;
                else
                    entry.sectionIndex = ELF::SectionOffsets::UNDEFINED;
            }

            insertSymbol64(entry, &symbol.name);
        }
    }

    // SYMTAB
    if (useELF32)
    {
        ELF::SectionHeader32 header;
        header.offsetInSectionNameStringTable = symtabNameOffset;
        header.type = ELF::SectionTypes::SYMTAB;
        header.flags = 0;

        if (sectionContentOffset > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Section offset in file too big for ELF32", 0, 0);
        header.offset = static_cast<uint32_t>(sectionContentOffset);

        if (symtabBuffer.size() > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Section size too big for ELF32", 0, 0);
        header.sectionSize = static_cast<uint32_t>(symtabBuffer.size());

        header.addressAlignment = 4;

        header.entrySize = sizeof(ELF::SymbolEntry32);

        if (strtabIndex > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("strtab index too big", 0, 0);
        header.linkIndex = static_cast<uint32_t>(strtabIndex);

        if (localSymbolsCount > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Too many local symbols", 0, 0);
        header.info = static_cast<uint32_t>(localSymbolsCount);

        header.virtualAddress = 0;

        WriteAll(&header, sizeof(header));
    }
    else
    {
        ELF::SectionHeader64 header;
        header.offsetInSectionNameStringTable = symtabNameOffset;
        header.type = ELF::SectionTypes::SYMTAB;
        header.flags = 0;

        header.offset = sectionContentOffset;
        header.sectionSize = static_cast<uint64_t>(symtabBuffer.size());
        header.addressAlignment = 8;

        header.entrySize = sizeof(ELF::SymbolEntry64);

        if (strtabIndex > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError(".strtab index too big", 0, 0);
        header.linkIndex = static_cast<uint32_t>(strtabIndex);

        if (localSymbolsCount > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Too many local symbols", 0, 0);
        header.info = static_cast<uint32_t>(localSymbolsCount);

        header.virtualAddress = 0;

        WriteAll(&header, sizeof(header));
    }
    sectionContentOffset += symtabBuffer.size();
    sectionContentOffset += (alignTo - (sectionContentOffset % alignTo)) % alignTo;

    // STRTAB
    if (useELF32)
    {
        ELF::SectionHeader32 header;
        header.offsetInSectionNameStringTable = strtabNameOffset;
        header.type = ELF::SectionTypes::STRTAB;
        header.flags = 0;

        if (sectionContentOffset > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Section offset in file too big for ELF32", 0, 0);
        header.offset = static_cast<uint32_t>(sectionContentOffset);

        if (strtabBuffer.size() > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Section size too big for ELF32", 0, 0);
        header.sectionSize = static_cast<uint32_t>(strtabBuffer.size());

        header.addressAlignment = 1;

        header.entrySize = 0;

        header.virtualAddress = 0;
        header.linkIndex = 0;
        header.info = 0;

        WriteAll(&header, sizeof(header));
    }
    else
    {
        ELF::SectionHeader64 header;
        header.offsetInSectionNameStringTable = strtabNameOffset;
        header.type = ELF::SectionTypes::STRTAB;
        header.flags = 0;

        header.offset = sectionContentOffset;
        header.sectionSize = static_cast<uint64_t>(strtabBuffer.size());
        header.addressAlignment = 1;

        header.entrySize = 0;

        header.virtualAddress = 0;
        header.linkIndex = 0;
        header.info = 0;

        WriteAll(&header, sizeof(header));
    }
    sectionContentOffset += strtabBuffer.size();
    sectionContentOffset += (alignTo - (sectionContentOffset % alignTo)) % alignTo;

    for (Object::Section& section : sections)
    {
        auto it = sectionIndices.find(section.name.str());
        if (it == sectionIndices.end())
            throw Exception::InternalError("Could not find section '" + section.name.str() + "'", 0, 0);
        
        if (it->second > std::numeric_limits<uint32_t>::max())
            throw Exception::OverflowError("Section index too big", 0, 0);
        const uint32_t idx = static_cast<uint32_t>(it->second);

        if (!section.relocations.empty())
        {
            if (relocationShstrtabIndices.empty())
                throw Exception::InternalError("Relocation section indices queue is empty", 0, 0);

            bool useAddend = !useELF32;

            for (const Object::Relocation& relocation : section.relocations)
            {
                if (useAddend)
                {
                    // TODO: check if actual useful
                    // If the relocation has an addend, this code removes it from the code itself
                    switch (relocation.size)
                    {
                        case Object::Relocation::Size::Bit8:
                        {
                            std::memset(section.buffer.data() + relocation.offsetInSection, 0, sizeof(uint8_t));
                            break;
                        }
                        case Object::Relocation::Size::Bit16:
                        {
                            std::memset(section.buffer.data() + relocation.offsetInSection, 0, sizeof(uint16_t));
                            break;
                        }
                        case Object::Relocation::Size::Bit24:
                        {
                            std::memset(section.buffer.data() + relocation.offsetInSection, 0, 3);
                            break;
                        }
                        case Object::Relocation::Size::Bit32:
                        {
                            std::memset(section.buffer.data() + relocation.offsetInSection, 0, sizeof(uint32_t));
                            break;
                        }
                        case Object::Relocation::Size::Bit64:
                        {
                            std::memset(section.buffer.data() + relocation.offsetInSection, 0, sizeof(uint64_t));
                            break;
                        }
                    }
                }
            }

            uint64_t relocCount = section.relocations.size();

            if (useELF32)
            {
                ELF::SectionHeader32 header;
                header.offsetInSectionNameStringTable = relocationShstrtabIndices.front();
                relocationShstrtabIndices.pop();
                header.type = useAddend ? ELF::SectionTypes::RELA : ELF::SectionTypes::REL;
                header.flags = 0;

                header.entrySize = useAddend ? sizeof(ELF::Relocation::Rela32) : sizeof(ELF::Relocation::Rel32);

                if (sectionContentOffset > std::numeric_limits<uint32_t>::max())
                    throw Exception::OverflowError("Section offset in file too big for ELF32", 0, 0);
                header.offset = static_cast<uint32_t>(sectionContentOffset);

                uint64_t relocSize = header.entrySize * relocCount;
                if (relocSize > std::numeric_limits<uint32_t>::max())
                    throw Exception::OverflowError("Section size too big for ELF32", 0, 0);
                header.sectionSize = static_cast<uint32_t>(relocSize);

                header.addressAlignment = 4;

                if (symtabIndex > std::numeric_limits<uint32_t>::max())
                    throw Exception::OverflowError(".symtab index too big", 0, 0);
                header.linkIndex = static_cast<uint32_t>(symtabIndex);

                header.info = idx;

                header.virtualAddress = 0;

                WriteAll(&header, sizeof(header));

                sectionContentOffset += header.sectionSize;
                sectionContentOffset += (alignTo - (sectionContentOffset % alignTo)) % alignTo;
            }
            else
            {
                ELF::SectionHeader64 header;
                header.offsetInSectionNameStringTable = relocationShstrtabIndices.front();
                relocationShstrtabIndices.pop();
                header.type = useAddend ? ELF::SectionTypes::RELA : ELF::SectionTypes::REL;
                header.flags = 0;

                header.entrySize = useAddend ? sizeof(ELF::Relocation::Rela64) : sizeof(ELF::Relocation::Rel64);

                header.offset = sectionContentOffset;

                uint64_t relocSize = header.entrySize * relocCount;
                header.sectionSize = relocSize;

                header.addressAlignment = 8;

                if (symtabIndex > std::numeric_limits<uint32_t>::max())
                    throw Exception::OverflowError(".symtab index too big", 0, 0);
                header.linkIndex = static_cast<uint32_t>(symtabIndex);

                header.info = idx;

                header.virtualAddress = 0;

                WriteAll(&header, sizeof(header));

                sectionContentOffset += header.sectionSize;
                sectionContentOffset += (alignTo - (sectionContentOffset % alignTo)) % alignTo;
            }
        }
    }

    WritePadding(alignTo);

    // SECTIONS
    for (const Object::Section& section : sections)
    {
        switch (section.type)
        {
            case Object::Section::Type::Initialized:
                WriteAll(section.buffer.data(), section.size);
                WritePadding(alignTo);
                break;

            case Object::Section::Type::Uninitialized:
                break;

            default:
                throw Exception::InternalError("Invalid section type", 0, 0);
        }
    }

    // SHSTRTAB
    WriteAll(shstrtabBuffer.data(), shstrtabBuffer.size());
    WritePadding(alignTo);

    // SYMTAB
    WriteAll(symtabBuffer.data(), symtabBuffer.size());
    WritePadding(alignTo);

    // STRTAB
    WriteAll(strtabBuffer.data(), strtabBuffer.size());
    WritePadding(alignTo);

    // RELOCATIONS
    for (const Object::Section& section : sections)
    {
        bool useAddend = !useELF32;

        for (const Object::Relocation& relocation : section.relocations)
        {
            uint32_t type;
            switch (instructionSet)
            {
                case ELF::InstructionSet::I386:
                    switch (relocation.type)
                    {
                        case Object::Relocation::Type::Absolute:
                            switch (relocation.size)
                            {
                                case Object::Relocation::Size::Bit8:
                                    type = ELF::Intel::Relocations::R386_8;
                                    break;

                                case Object::Relocation::Size::Bit16:
                                    type = ELF::Intel::Relocations::R386_16;
                                    break;

                                case Object::Relocation::Size::Bit24:
                                    throw Exception::FormatError("24-bit relocations are not supported with i386 ELF", 0, 0);

                                case Object::Relocation::Size::Bit32:
                                    type = ELF::Intel::Relocations::R386_32;
                                    break;

                                case Object::Relocation::Size::Bit64:
                                    context.warningManager->add(Warning::FormatWarning("64-bit relocations are not supported with i386 ELF: Using 32-bit instead", 0, 0, nullptr));
                                    type = ELF::Intel::Relocations::R386_32;
                                    break;

                                default:
                                    throw Exception::InternalError("Invalid relocation size", 0, 0);
                            }
                            break;

                        case Object::Relocation::Type::PC_Relative:
                            switch (relocation.size)
                            {
                                case Object::Relocation::Size::Bit8:
                                    type = ELF::Intel::Relocations::R386_PC8;
                                    break;

                                case Object::Relocation::Size::Bit16:
                                    type = ELF::Intel::Relocations::R386_PC16;
                                    break;

                                case Object::Relocation::Size::Bit24:
                                    throw Exception::FormatError("24-bit relocations are not supported with i386 ELF", 0, 0);

                                case Object::Relocation::Size::Bit32:
                                    type = ELF::Intel::Relocations::R386_PC32;
                                    break;

                                case Object::Relocation::Size::Bit64:
                                    context.warningManager->add(Warning::FormatWarning("64-bit relocations are not supported with i386 ELF: Using 32-bit instead", 0, 0, nullptr));
                                    type = ELF::Intel::Relocations::R386_PC32;
                                    break;

                                default:
                                    throw Exception::InternalError("Invalid relocation size", 0, 0);
                            }
                            break;

                        default:
                            throw Exception::InternalError("Invalid relocation type", 0, 0);
                    }
                    break;

                case ELF::InstructionSet::X86_64:
                    switch (relocation.type)
                    {
                        case Object::Relocation::Type::Absolute:
                            switch (relocation.size)
                            {
                                case Object::Relocation::Size::Bit8:
                                    type = ELF::AMD::Relocations::R64_8;
                                    break;

                                case Object::Relocation::Size::Bit16:
                                    type = ELF::AMD::Relocations::R64_16;
                                    break;

                                case Object::Relocation::Size::Bit24:
                                    throw Exception::FormatError("24-bit relocations are not supported with x86_64 ELF", 0, 0);

                                case Object::Relocation::Size::Bit32:
                                    if (relocation.isSigned)
                                        type = ELF::AMD::Relocations::R64_32S;
                                    else
                                        type = ELF::AMD::Relocations::R64_32;
                                    break;

                                case Object::Relocation::Size::Bit64:
                                    type = ELF::AMD::Relocations::R64_64;
                                    break;

                                default:
                                    throw Exception::InternalError("Invalid relocation size", 0, 0);
                            }
                            break;

                        case Object::Relocation::Type::PC_Relative:
                            switch (relocation.size)
                            {
                                case Object::Relocation::Size::Bit8:
                                    type = ELF::AMD::Relocations::R64_PC8;
                                    break;

                                case Object::Relocation::Size::Bit16:
                                    type = ELF::AMD::Relocations::R64_PC16;
                                    break;

                                case Object::Relocation::Size::Bit24:
                                    throw Exception::FormatError("24-bit relocations are not supported with x86_64 ELF", 0, 0);

                                case Object::Relocation::Size::Bit32:
                                    type = ELF::AMD::Relocations::R64_PC32;
                                    break;

                                case Object::Relocation::Size::Bit64:
                                    type = ELF::AMD::Relocations::R64_PC64;
                                    break;

                                default:
                                    throw Exception::InternalError("Invalid relocation size", 0, 0);
                            }
                            break;

                        default:
                            throw Exception::InternalError("Invalid relocation type", 0, 0);
                    }
                    break;

                case ELF::InstructionSet::ARM:
                case ELF::InstructionSet::AARCH64:
                    throw Exception::InternalError("ARM not supported yet", 0, 0);

                case ELF::InstructionSet::RISCV:
                    throw Exception::InternalError("RISC-V not supported yet", 0, 0);

                default:
                    throw Exception::InternalError("Invalid architecture", 0, 0);
            }

            auto it = symbolIndices.find(relocation.usedSection.str());
            if (it == symbolIndices.end())
                throw Exception::InternalError("Could not find index of symbol '" + relocation.usedSection.str() + "'", 0, 0);
            uint64_t symbolIndex = it->second;

            if (symbolIndex > std::numeric_limits<uint32_t>::max())
                throw Exception::OverflowError("Index of symbol '" + relocation.usedSection.str() + "' too big", 0, 0);
            uint32_t sym = static_cast<uint32_t>(symbolIndex);

            if (useELF32)
            {
                if (relocation.offsetInSection > std::numeric_limits<uint32_t>::max())
                    throw Exception::OverflowError("Offset for relocation too big for ELF32", 0, 0);
                
                if (relocation.addend > std::numeric_limits<int32_t>::max())
                    throw Exception::OverflowError("Addend for relocation too big for ELF32", 0, 0);
                if (relocation.addend < std::numeric_limits<int32_t>::min())
                    throw Exception::OverflowError("Addend for relocation too small for ELF32", 0, 0);

                if (type > std::numeric_limits<uint8_t>::max())
                    throw Exception::OverflowError("Type too big for ELF32", 0, 0);

                if (sym > 16777215) // std::numeric_limits<uint24_t>::max()
                    throw Exception::OverflowError("Symbol index too big for ELF32", 0, 0);

                if (useAddend)
                {
                    ELF::Relocation::Rela32 reloc;
                    reloc.offsetInSection = static_cast<uint32_t>(relocation.offsetInSection);
                    reloc.addend = static_cast<int32_t>(relocation.addend);
                    reloc.info = ELF::Relocation::GetInfo32(sym, static_cast<uint8_t>(type));
                    WriteAll(&reloc, sizeof(reloc));
                }
                else
                {
                    ELF::Relocation::Rel32 reloc;
                    reloc.offsetInSection = static_cast<uint32_t>(relocation.offsetInSection);
                    reloc.info = ELF::Relocation::GetInfo32(sym, static_cast<uint8_t>(type));
                    WriteAll(&reloc, sizeof(reloc));
                }
            }
            else
            {
                if (useAddend)
                {
                    ELF::Relocation::Rela64 reloc;
                    reloc.offsetInSection = relocation.offsetInSection;
                    reloc.addend = relocation.addend;
                    reloc.info = ELF::Relocation::GetInfo64(sym, type);
                    WriteAll(&reloc, sizeof(reloc));
                }
                else
                {
                    ELF::Relocation::Rel64 reloc;
                    reloc.offsetInSection = relocation.offsetInSection;
                    reloc.info = ELF::Relocation::GetInfo64(sym, type);
                    WriteAll(&reloc, sizeof(reloc));
                }
            }
        }

        WritePadding(alignTo);
    }
}
