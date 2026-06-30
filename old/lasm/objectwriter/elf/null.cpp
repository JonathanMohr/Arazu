#include "elf.hpp"

#include "header.hpp"

void Assembler::ELFWriter::WriteNullSectionHeader()
{
    if (useELF32)
    {
        ELF::SectionHeader32 nullSection;
        nullSection.offsetInSectionNameStringTable = 0;
        nullSection.type                           = 0;
        nullSection.flags                          = 0;
        nullSection.virtualAddress                 = 0;
        nullSection.offset                         = 0;
        nullSection.sectionSize                    = 0;
        nullSection.linkIndex                      = 0;
        nullSection.info                           = 0;
        nullSection.addressAlignment               = 0;
        nullSection.entrySize                      = 0;

        WriteAll(&nullSection, sizeof(nullSection));
    }
    else
    {
        ELF::SectionHeader64 nullSection;
        nullSection.offsetInSectionNameStringTable = 0;
        nullSection.type                           = 0;
        nullSection.flags                          = 0;
        nullSection.virtualAddress                 = 0;
        nullSection.offset                         = 0;
        nullSection.sectionSize                    = 0;
        nullSection.linkIndex                      = 0;
        nullSection.info                           = 0;
        nullSection.addressAlignment               = 0;
        nullSection.entrySize                      = 0;

        WriteAll(&nullSection, sizeof(nullSection));
    }
}
