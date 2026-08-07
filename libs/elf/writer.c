#include "arazu/core/context.h"
#include "arazu/core/object/object.h"
#include "arazu/core/object/relocation.h"
#include "arazu/core/object/section.h"
#include "arazu/core/object/symbol.h"
#include "arazu/core/types.h"
#include <arazu/elf/writer.h>

#include <support.h>

#include <elf.h>
#include <endianness.h>

#define BUFFER_PUSH_FUNCTION(type) \
    static inline void Buffer_Push_##type(Arazu_u8** ptr, Arazu_##type val) \
    { \
        memcpy(*ptr, &val, sizeof(val)); \
        *ptr += sizeof(val); \
    }

BUFFER_PUSH_FUNCTION(u8)
BUFFER_PUSH_FUNCTION(u16)
BUFFER_PUSH_FUNCTION(u32)
BUFFER_PUSH_FUNCTION(u64)

static Arazu_Bool Arazu_ELF_WriteSectionHeader(
    const Arazu_Context* ctx,
    Arazu_ELF_FileWriter* fileWriter,
    Arazu_Bool isLittleEndian,
    Arazu_Bool is64,
    Arazu_u32 name,
    Arazu_u32 type,
    Arazu_u64 flags,
    Arazu_u64 address,
    Arazu_u64 offset,
    Arazu_u64 size,
    Arazu_u32 link,
    Arazu_u32 info,
    Arazu_u64 alignment,
    Arazu_u64 entrySize
)
{
    (void)ctx;

    Arazu_u8 buffer[ELF_SECTIONHEADER_SIZE];
    Arazu_u8* bufferPtr = buffer;

    Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, name));
    Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, type));

    if (is64 == ARAZU_TRUE)
    {
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, flags));
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, address));
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, offset));
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, size));
    }
    else
    {
        if (flags > 0xFFFFFFFF) return ARAZU_FALSE;
        if (address > 0xFFFFFFFF) return ARAZU_FALSE;
        if (offset > 0xFFFFFFFF) return ARAZU_FALSE;
        if (size > 0xFFFFFFFF) return ARAZU_FALSE;

        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, (Arazu_u32)flags));
        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, (Arazu_u32)address));
        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, (Arazu_u32)offset));
        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, (Arazu_u32)size));
    }

    Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, link));
    Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, info));

    if (is64 == ARAZU_TRUE)
    {
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, alignment));
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, entrySize));
    }
    else
    {
        if (alignment > 0xFFFFFFFF) return ARAZU_FALSE;
        if (entrySize > 0xFFFFFFFF) return ARAZU_FALSE;

        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, (Arazu_u32)alignment));
        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, (Arazu_u32)entrySize));
    }

    if (fileWriter->write(fileWriter, buffer, (is64 == ARAZU_TRUE) ? ELF_SECTIONHEADER_SIZE_64 : ELF_SECTIONHEADER_SIZE_32) != ARAZU_TRUE)
        return ARAZU_FALSE;

    return ARAZU_TRUE;
}

static void write_bits(Arazu_u8* buffer, Arazu_Size bitOffset, Arazu_Size bitCount, Arazu_Value value, Arazu_Bool isLittleEndian)
{
    for (Arazu_Size i = 0; i < bitCount; i++)
    {
        Arazu_Size pos = bitOffset + i;
        Arazu_Size byteIndex = pos / 8;
        Arazu_u8 bitInByteIndex = pos % 8;

        Arazu_u64 destBitPos;
        if (isLittleEndian == ARAZU_TRUE)
            destBitPos = bitInByteIndex;
        else
            destBitPos = 7 - bitInByteIndex;

        Arazu_u8 bitValue = (Arazu_u8)((value >> (bitCount - 1 - i)) & 0x1);

        if (bitValue)
            buffer[byteIndex] |= (Arazu_u8)(1u << destBitPos);
        else
            buffer[byteIndex] &= (Arazu_u8)~(1u << destBitPos);
    }
}

Arazu_Bool Arazu_ELF_WriteObject(const Arazu_Context* ctx, const Arazu_Object* object, Arazu_ELF_FileWriter* fileWriter)
{
    Arazu_Bool isLittleEndian = ARAZU_FALSE; /* TODO: Set dynamically */
    Arazu_Bool useAddend = ARAZU_TRUE; /* TODO: Set dynamically */

    Arazu_Size sectionNameLengths = 1 + 8 + 8 + 10; /* null, .symtab, .strtab, .shstrtab */
    Arazu_Size symbolNameLengths = 1; /* null */

    Arazu_uValue symbolCount = 1 + Arazu_Object_GetSymbolCount(ctx, object); /* null */
    Arazu_uValue localSymbolCount = 1; /* null */

    for (Arazu_uValue i = 0; i < symbolCount - 1; i++)
    {
        const Arazu_Object_Symbol* symbol = Arazu_Object_GetSymbol(ctx, object, i);
        if (Arazu_Object_Symbol_GetVisibility(ctx, symbol))
            localSymbolCount++;

        const Arazu_String symbolName = Arazu_Object_Symbol_GetName(ctx, symbol);
        const char* symbolNameStr = Arazu_Context_GetStringPool(ctx)->toCString(Arazu_Context_GetStringPool(ctx), symbolName);

        while (*symbolNameStr)
        {
            symbolNameLengths++;
            symbolNameStr++;
        }
        symbolNameLengths++;
    }

    Arazu_uValue sectionCount = 4; /* null, .symtab, .strtab, .shstrtab */

    Arazu_Size elfAlignment = 16;

    for (Arazu_uValue i = 0; i < Arazu_Object_GetSectionCount(ctx, object); i++)
    {
        const Arazu_Object_Section* section = Arazu_Object_GetSection(ctx, object, i);
        const Arazu_String name = Arazu_Object_Section_GetName(ctx, section);
        const char* nameStr = Arazu_Context_GetStringPool(ctx)->toCString(Arazu_Context_GetStringPool(ctx), name);

        Arazu_uValue sectionSymbolCount = Arazu_Object_Section_GetSymbolCount(ctx, section);
        for (Arazu_uValue j = 0; j < sectionSymbolCount; j++)
        {
            const Arazu_Object_Symbol* symbol = Arazu_Object_Section_GetSymbol(ctx, section, j);
            if (Arazu_Object_Symbol_GetVisibility(ctx, symbol))
                localSymbolCount++;
        }

        Arazu_Size sectionNameLength = 0;
        while (*nameStr)
        {
            sectionNameLength++;
            nameStr++;
        }

        sectionNameLengths += sectionNameLength + 1;

        sectionCount++;

        if (Arazu_Object_Section_GetRelocationCount(ctx, section) > 0)
        {
            sectionNameLengths += sectionNameLength + ((useAddend == ARAZU_TRUE) ? 6 : 5);
            sectionCount++; /* relocation section */
        }

        symbolCount += Arazu_Object_Section_GetSymbolCount(ctx, section);
    }

    const Arazu_uValue shstrtabIndex = sectionCount - 1;
    const Arazu_uValue strtabIndex = sectionCount - 2;
    const Arazu_uValue symtabIndex = sectionCount - 3;


    Arazu_u8 buffer[ELF_HEADER_SIZE] = {0};
    Arazu_u8* bufferPtr = buffer;

    Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_MAGIC_0));
    Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_MAGIC_1));
    Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_MAGIC_2));
    Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_MAGIC_3));

    Arazu_Bool is64 = ARAZU_FALSE;
    switch (Arazu_Object_GetBitMode(ctx, object))
    {
        case 32:
            Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_CLASS_32));
            break;

        case 64:
            Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_CLASS_64));
            is64 = ARAZU_TRUE;
            break;

        default:
            /* TODO: Maybe give info */
            return ARAZU_FALSE;
    }

    if (isLittleEndian)
        Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_ENDIANNESS_LITTLE));
    else
        Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_ENDIANNESS_BIG));

    Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_VERSION));

    /* TODO */
    Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_OSABI_NONE));
    Buffer_Push_u8(&bufferPtr, Arazu_Endianness_Convert_u8(isLittleEndian, ELF_HEADER_IDENT_ABIVERSION_UNSPECIFIED));

    /* 7 bytes padding */
    bufferPtr += 7;

    
    /* type */
    Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, ELF_HEADER_TYPE_RELOCATABLE));

    /* machine */
    Arazu_u16 machine;
    switch (Arazu_Object_GetArchitecture(ctx, object))
    {
        case ARAZU_ARCHITECTURE_X86:
            if (is64 == ARAZU_TRUE)
                machine = ELF_HEADER_MACHINE_X86_64;
            else
                machine = ELF_HEADER_MACHINE_386;
            break;

        default:
            machine = ELF_HEADER_MACHINE_NONE;
            break;
    }
    Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, machine));

    /* version */
    Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, ELF_HEADER_VERSION_CURRENT));


    if (is64)
    {
        /* entry */
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, 0));

        /* programHeaderTableOffset */
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, 0));

        /* sectionHeaderTableOffset */
        Buffer_Push_u64(&bufferPtr, Arazu_Endianness_Convert_u64(isLittleEndian, ELF_HEADER_SIZE));
    }
    else
    {
        /* entry */
        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, 0));

        /* programHeaderTableOffset */
        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, 0));

        /* sectionHeaderTableOffset */
        Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, ELF_HEADER_SIZE));
    }

    /* flags */
    Buffer_Push_u32(&bufferPtr, Arazu_Endianness_Convert_u32(isLittleEndian, 0)); /* TODO */

    /* headerSize */
    if (is64)
        Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, ELF_HEADER_SIZE_64));
    else
        Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, ELF_HEADER_SIZE_32));

    /* programHeaderSize */
    Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, 0)); /* TODO: maybe set it */
    
    /* programHeaderCount */
    Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, 0));

    /* sectionHeaderSize */
    if (is64)
        Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, ELF_SECTIONHEADER_SIZE_64));
    else
        Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, ELF_SECTIONHEADER_SIZE_32));

    /* sectionHeaderCount */
    Arazu_Bool tooManySections = ARAZU_FALSE;

    if (sectionCount < 0xFF00)
    {
        Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, (Arazu_u16)sectionCount));
    }
    else
    {
        Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, 0));
        tooManySections = ARAZU_TRUE;
    }

    /* sectionNameStringTableIndex (TODO: set it to the actual value) */
    Arazu_Bool shstrtabIndexTooHigh = ARAZU_FALSE;

    if (shstrtabIndex < 0xFF00)
        Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, (Arazu_u16)shstrtabIndex));
    else
    {
        Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, 0xFFFF));
        shstrtabIndexTooHigh = ARAZU_TRUE;
    }


    if (fileWriter->write(fileWriter, buffer, ELF_HEADER_SIZE) != ARAZU_TRUE)
        return ARAZU_FALSE;

    if (shstrtabIndex > 0xFFFFFFFF)
        return ARAZU_FALSE;

    /* null section header */
    if (Arazu_ELF_WriteSectionHeader(
        ctx, fileWriter, isLittleEndian, is64,
        0,
        ELF_SECTIONHEADER_TYPE_NULL,
        0,
        0,
        0,
        (tooManySections == ARAZU_TRUE) ? sectionCount : 0,
        (shstrtabIndexTooHigh == ARAZU_TRUE) ? (Arazu_u32)shstrtabIndex : 0,
        0,
        0,
        0
    ) != ARAZU_TRUE) return ARAZU_FALSE;

    const Arazu_Size startFileOffset = ELF_HEADER_SIZE + sectionCount * ((is64 == ARAZU_TRUE) ? ELF_SECTIONHEADER_SIZE_64 : ELF_SECTIONHEADER_SIZE_32);
    Arazu_Size currentFileOffset = startFileOffset;
    currentFileOffset = ((currentFileOffset + elfAlignment - 1) / elfAlignment) * elfAlignment;
    const Arazu_Size sectionHeaderPadding = currentFileOffset - startFileOffset;

    Arazu_Size currentSectionNameOffset = 1;

    Arazu_Size currentSectionIndex = 1;

    Arazu_uValue biggestSectionSize = 0;

    for (Arazu_uValue i = 0; i < Arazu_Object_GetSectionCount(ctx, object); i++)
    {
        const Arazu_Object_Section* section = Arazu_Object_GetSection(ctx, object, i);
        const Arazu_String name = Arazu_Object_Section_GetName(ctx, section);
        const char* nameStr = Arazu_Context_GetStringPool(ctx)->toCString(Arazu_Context_GetStringPool(ctx), name);

        if (currentSectionNameOffset > 0xFFFFFFFF)
            return ARAZU_FALSE; /* TODO */

        Arazu_uValue size = Arazu_Object_Section_GetSize(ctx, section);
        if (size > biggestSectionSize)
            biggestSectionSize = size;

        Arazu_u32 type;
        switch (Arazu_Object_Section_GetType(ctx, section))
        {
            case ARAZU_OBJECT_SECTION_TYPE_INITIALIZED:
                type = ELF_SECTIONHEADER_TYPE_PROGBITS;
                break;

            case ARAZU_OBJECT_SECTION_TYPE_UNINITIALIZED:
                type = ELF_SECTIONHEADER_TYPE_NOBITS;
                break;

            default:
                type = ELF_SECTIONHEADER_TYPE_NULL;
                break;
        }

        Arazu_u32 flags = 0;
        Arazu_Object_Section_Flags sectionFlags = Arazu_Object_Section_GetFlags(ctx, section);
        if (sectionFlags & ARAZU_OBJECT_SECTION_FLAGS_ALLOCATED)
            flags |= ELF_SECTIONHEADER_FLAGS_ALLOC;
        if (sectionFlags & ARAZU_OBJECT_SECTION_FLAGS_EXECUTABLE)
            flags |= ELF_SECTIONHEADER_FLAGS_EXECINSTR;
        if (sectionFlags & ARAZU_OBJECT_SECTION_FLAGS_WRITABLE)
            flags |= ELF_SECTIONHEADER_FLAGS_WRITE;

        if (Arazu_ELF_WriteSectionHeader(
            ctx, fileWriter, isLittleEndian, is64,
            (Arazu_u32)currentSectionNameOffset,
            type,
            flags,
            0,
            currentFileOffset,
            size,
            0, /* TODO */
            0, /* TODO */
            Arazu_Object_Section_GetAlign(ctx, section),
            0
        ) != ARAZU_TRUE) return ARAZU_FALSE;
        currentSectionIndex++;

        currentFileOffset += size;
        currentFileOffset = ((currentFileOffset + elfAlignment - 1) / elfAlignment) * elfAlignment;

        Arazu_Size sectionNameLength = 0;
        while (*nameStr)
        {
            sectionNameLength++;
            nameStr++;
        }
        currentSectionNameOffset += sectionNameLength + 1;

        Arazu_uValue relocationCount = Arazu_Object_Section_GetRelocationCount(ctx, section);
        if (relocationCount > 0)
        {
            if (currentSectionNameOffset > 0xFFFFFFFF)
                return ARAZU_FALSE; /* TODO */

            if (symtabIndex > 0xFFFFFFFF)
                return ARAZU_FALSE; /* TODO */

            if (currentSectionIndex > 0xFFFFFFFF)
                return ARAZU_FALSE; /* TODO */

            const Arazu_Size entrySize = (is64 == ARAZU_TRUE)
             ? ((useAddend == ARAZU_TRUE) ? ELF_RELOCATION_RELA_SIZE64 : ELF_RELOCATION_REL_SIZE64)
             : ((useAddend == ARAZU_TRUE) ? ELF_RELOCATION_RELA_SIZE32 : ELF_RELOCATION_REL_SIZE32);

            if (Arazu_ELF_WriteSectionHeader(
                ctx, fileWriter, isLittleEndian, is64,
                (Arazu_u32)currentSectionNameOffset,
                (useAddend == ARAZU_TRUE) ? ELF_SECTIONHEADER_TYPE_RELA : ELF_SECTIONHEADER_TYPE_REL,
                0,
                0,
                currentFileOffset,
                relocationCount * entrySize,
                (Arazu_u32)symtabIndex,
                (Arazu_u32)currentSectionIndex,
                4,
                entrySize
            ) != ARAZU_TRUE) return ARAZU_FALSE;
            currentSectionIndex++;

            currentFileOffset += relocationCount * entrySize;
            currentFileOffset = ((currentFileOffset + elfAlignment - 1) / elfAlignment) * elfAlignment;

            currentSectionNameOffset += sectionNameLength + ((useAddend == ARAZU_TRUE) ? 6 : 5);
        }
    }

    // SYMTAB
    if (currentSectionNameOffset > 0xFFFFFFFF)
        return ARAZU_FALSE; /* TODO */

    if (localSymbolCount > 0xFFFFFFFF)
        return ARAZU_FALSE; /* TODO */

    if (strtabIndex > 0xFFFFFFFF)
        return ARAZU_FALSE; /* TODO */

    const Arazu_Size symtabEntrySize = 0; /* TODO */

    if (Arazu_ELF_WriteSectionHeader(
        ctx, fileWriter, isLittleEndian, is64,
        (Arazu_u32)currentSectionNameOffset,
        ELF_SECTIONHEADER_TYPE_SYMTAB,
        0,
        0,
        currentFileOffset,
        symbolCount * symtabEntrySize,
        (Arazu_u32)strtabIndex,
        (Arazu_u32)localSymbolCount,
        4,
        symtabEntrySize
    ) != ARAZU_TRUE) return ARAZU_FALSE;
    
    currentSectionNameOffset += 8;
    currentFileOffset += symbolCount * symtabEntrySize;
    currentFileOffset = ((currentFileOffset + elfAlignment - 1) / elfAlignment) * elfAlignment;

    // STRTAB
    if (currentSectionNameOffset > 0xFFFFFFFF)
        return ARAZU_FALSE; /* TODO */

    if (Arazu_ELF_WriteSectionHeader(
        ctx, fileWriter, isLittleEndian, is64,
        (Arazu_u32)currentSectionNameOffset,
        ELF_SECTIONHEADER_TYPE_STRTAB,
        0,
        0,
        currentFileOffset,
        symbolNameLengths,
        0,
        0,
        1,
        0
    ) != ARAZU_TRUE) return ARAZU_FALSE;
    
    currentSectionNameOffset += 8;
    currentFileOffset += symbolNameLengths;
    currentFileOffset = ((currentFileOffset + elfAlignment - 1) / elfAlignment) * elfAlignment;

    // SHSTRTAB
    if (currentSectionNameOffset > 0xFFFFFFFF)
        return ARAZU_FALSE; /* TODO */

    if (Arazu_ELF_WriteSectionHeader(
        ctx, fileWriter, isLittleEndian, is64,
        (Arazu_u32)currentSectionNameOffset,
        ELF_SECTIONHEADER_TYPE_STRTAB,
        0,
        0,
        currentFileOffset,
        sectionNameLengths,
        0,
        0,
        1,
        0
    ) != ARAZU_TRUE) return ARAZU_FALSE;
    
    currentSectionNameOffset += 10;
    currentFileOffset += sectionNameLengths;
    currentFileOffset = ((currentFileOffset + elfAlignment - 1) / elfAlignment) * elfAlignment;

    Arazu_u8 paddingBuffer[16] = {0};
    Arazu_Size paddingRemaining = sectionHeaderPadding;
    while (paddingRemaining > 0)
    {
        const Arazu_Size chunk = (sizeof(paddingBuffer) < paddingRemaining) ? sizeof(paddingBuffer) : paddingRemaining;

        if (fileWriter->write(fileWriter, paddingBuffer, chunk) != ARAZU_TRUE)
            return ARAZU_FALSE;

        paddingRemaining -= chunk;
    }


    Arazu_u8* sectionBuffer = Arazu_Context_GetAllocator(ctx)->allocate(Arazu_Context_GetAllocator(ctx), biggestSectionSize);
    if (!sectionBuffer)
        return ARAZU_FALSE;

    for (Arazu_uValue i = 0; i < Arazu_Object_GetSectionCount(ctx, object); i++)
    {
        const Arazu_Object_Section* section = Arazu_Object_GetSection(ctx, object, i);
        const Arazu_Size sectionSize = Arazu_Object_Section_GetSize(ctx, section);
        const Arazu_u8* constSectionBuffer = Arazu_Object_Section_GetBuffer(ctx, section);

        for (Arazu_Size j = 0; j < sectionSize; j++)
            sectionBuffer[j] = constSectionBuffer[j];

        const Arazu_uValue relocationCount = Arazu_Object_Section_GetRelocationCount(ctx, section);

        if (useAddend != ARAZU_TRUE)
        {
            for (Arazu_uValue j = 0; j < relocationCount; j++)
            {
                const Arazu_Object_Relocation* relocation = Arazu_Object_Section_GetRelocation(ctx, section, j);

                const Arazu_uValue offsetInSection = Arazu_Object_Relocation_GetOffsetInSection(ctx, relocation);

                const Arazu_uValue bitCount = Arazu_Object_Relocation_GetSize(ctx, relocation);
                const Arazu_uValue bitOffset = 0; /* TODO */
                
                if (sectionSize < (offsetInSection + ((bitOffset + bitCount + 7) / 8)))
                {
                    Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), sectionBuffer);
                    return ARAZU_FALSE;
                }
                
                write_bits(sectionBuffer + offsetInSection, bitOffset, bitCount, Arazu_Object_Relocation_GetAddend(ctx, relocation), isLittleEndian);
            }
        }

        if (fileWriter->write(fileWriter, sectionBuffer, sectionSize) != ARAZU_TRUE)
        {
            Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), sectionBuffer);
            return ARAZU_FALSE;
        }

        paddingRemaining = (elfAlignment - sectionSize % elfAlignment) % elfAlignment;
        while (paddingRemaining > 0)
        {
            const Arazu_Size chunk = (sizeof(paddingBuffer) < paddingRemaining) ? sizeof(paddingBuffer) : paddingRemaining;

            if (fileWriter->write(fileWriter, paddingBuffer, chunk) != ARAZU_TRUE)
                return ARAZU_FALSE;

            paddingRemaining -= chunk;
        }

        for (Arazu_uValue j = 0; j < relocationCount; j++)
        {
            /* TODO */
        }
    }

    Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), sectionBuffer);


    return ARAZU_TRUE;
}
