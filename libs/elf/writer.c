#include <arazu/elf/writer.h>

#include <support.h>

#include <elf.h>
#include <endianness.h>

static inline void Buffer_Push_u8(Arazu_u8** ptr, Arazu_u8 val)
{
    **ptr = val;
    (*ptr)++;
}

static inline void Buffer_Push_u16(Arazu_u8** ptr, Arazu_u16 val)
{
    memcpy(*ptr, &val, sizeof(val));
    *ptr += sizeof(val);
}

Arazu_Bool Arazu_ELF_WriteObject(const Arazu_Context* ctx, const Arazu_Object* object, Arazu_ELF_FileWriter* fileWriter)
{
    Arazu_Bool isLittleEndian = ARAZU_TRUE; /* TODO: Set dynamically */

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

    
    Buffer_Push_u16(&bufferPtr, Arazu_Endianness_Convert_u16(isLittleEndian, ELF_HEADER_TYPE_RELOCATABLE));

    /* TODO: fill the buffer */

    if (fileWriter->write(fileWriter, buffer, ELF_HEADER_SIZE) != ARAZU_TRUE)
    {
        return ARAZU_FALSE;
    }

    (void)is64;

    return ARAZU_FALSE;
}
