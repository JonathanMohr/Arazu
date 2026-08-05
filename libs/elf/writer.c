#include "arazu/core/object/object.h"
#include "arazu/core/object/section.h"
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

Arazu_Bool Arazu_ELF_WriteObject(const Arazu_Context* ctx, const Arazu_Object* object, Arazu_ELF_FileWriter* fileWriter)
{
    Arazu_Bool isLittleEndian = ARAZU_TRUE; /* TODO: Set dynamically */
    Arazu_Bool useAddend = ARAZU_TRUE; /* TODO: Set dynamically */

    Arazu_Size sectionNameLength = 1 + 8 + 8 + 10; /* null, .symtab, .strtab, .shstrtab */

    Arazu_uValue sectionCount = 4; /* null, .symtab, .strtab, .shstrtab */

    for (Arazu_uValue i = 0; i < Arazu_Object_GetSectionCount(ctx, object); i++)
    {
        const Arazu_Object_Section* section = Arazu_Object_GetSection(ctx, object, i);

        /* TODO: sectionNameLength */

        sectionCount++;

        if (Arazu_Object_Section_GetRelocationCount(ctx, section) > 0)
            sectionCount++; /* relocation section */
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

    (void)strtabIndex;
    (void)symtabIndex;
    (void)useAddend;
    (void)sectionNameLength;

    /* TODO: Other section headers */

    return ARAZU_TRUE;
}
