#include <stdio.h>
#include <stdint.h>

#include <arazu/core/context.h>
#include <arazu/core/object/object.h>

#include <arazu/elf/writer.h>

#include "allocator.h"
#include "string_pool.h"

static Arazu_Bool ELFFileWriter_Write(Arazu_ELF_FileWriter* writer, const Arazu_u8* data, Arazu_Size size)
{
    FILE* file = (FILE*)writer->userdata;

    if (size > SIZE_MAX) // TODO
        return ARAZU_FALSE;
    
    size_t s = (size_t)size;

    if (fwrite(data, s, 1, file) != 1)
        return ARAZU_FALSE;

    return ARAZU_TRUE;
}

static Arazu_Bool createELFFileWriter(Arazu_ELF_FileWriter* out, const char* filename)
{
#ifdef _WIN32
    FILE* file;
    errno_t fileErr = fopen_s(&file, filename, "wb");
    if (fileErr != 0)
#else
    FILE* file = fopen(filename, "wb");
    if (!file)
#endif
    {
        return ARAZU_FALSE;
    }

    out->userdata = file;
    out->write = ELFFileWriter_Write;

    return ARAZU_TRUE;
}

static void destroyElfFileWriter(Arazu_ELF_FileWriter* writer)
{
    FILE* file = (FILE*)writer->userdata;
    fclose(file);
}

int main(int argc, const char* argv[])
{
    const char* filename = "output_arasm.o";

    (void)argc;
    (void)argv;

    Arazu_Allocator allocator = make_allocator();
    Arazu_StringPool stringPool = make_string_pool(0);

    if (stringPool.userdata == ARAZU_NULL)
    {
        fputs("Failed to create string pool\n", stderr);

        return 1;
    }
    Arazu_Context* context = Arazu_Context_Create(&allocator, &stringPool);
    if (context == ARAZU_NULL)
    {
        fputs("Failed to create Arazu context\n", stderr);

        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object* object = Arazu_Context_GetAllocator(context)->allocate(Arazu_Context_GetAllocator(context), Arazu_Object_Size());
    if (object == ARAZU_NULL)
    {
        fputs("Failed to allocate memory for object\n", stderr);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Create(object, context, ARAZU_ARCHITECTURE_X86, 32) != ARAZU_TRUE)
    {
        fputs("Failed to create object\n", stderr);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_ReserveSectionCount(context, object, 2) != ARAZU_TRUE)
    {
        fputs("Failed to reserve 2 sections in object\n", stderr);

        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }


    Arazu_Object_Section* section = Arazu_Context_GetAllocator(context)->allocate(Arazu_Context_GetAllocator(context), Arazu_Object_Section_Size());
    if (section == ARAZU_NULL)
    {
        fputs("Failed to allocate memory for section\n", stderr);

        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object_Symbol* symbol = Arazu_Context_GetAllocator(context)->allocate(Arazu_Context_GetAllocator(context), Arazu_Object_Symbol_Size());
    if (symbol == ARAZU_NULL)
    {
        fputs("Failed to allocate memory for symbol\n", stderr);

        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object_Relocation* relocation = Arazu_Context_GetAllocator(context)->allocate(Arazu_Context_GetAllocator(context), Arazu_Object_Relocation_Size());
    if (relocation == ARAZU_NULL)
    {
        fputs("Failed to allocate memory for relocation\n", stderr);

        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }


    // .text

    if (Arazu_Object_Section_Create(section, context, stringPool.intern(&stringPool, ".text"), 8, ARAZU_OBJECT_SECTION_FLAGS_ALLOCATED | ARAZU_OBJECT_SECTION_FLAGS_EXECUTABLE, ARAZU_OBJECT_SECTION_TYPE_INITIALIZED) != ARAZU_TRUE)
    {
        fputs("Failed to create .text section\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_ReserveBufferSize(context, section, 9) != ARAZU_TRUE)
    {
        fputs("Failed to reserve 9 bytes for .text\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_ReserveSymbolCount(context, section, 2) != ARAZU_TRUE)
    {
        fputs("Failed to reserve 2 symbol for .text\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_ReserveRelocationCount(context, section, 1) != ARAZU_TRUE)
    {
        fputs("Failed to reserve 1 relocation for .text\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    // mov eax, 0x00 ; relocation
    (void)Arazu_Object_Section_PushByte(context, section, 0xB8);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);

    // inc [eax + 4]
    (void)Arazu_Object_Section_PushByte(context, section, 0xFF);
    (void)Arazu_Object_Section_PushByte(context, section, 0x40);
    (void)Arazu_Object_Section_PushByte(context, section, 0x04);

    // ret
    (void)Arazu_Object_Section_PushByte(context, section, 0xC3);

    if (Arazu_Object_Symbol_Create(symbol, context, 0, stringPool.intern(&stringPool, "test_function"), stringPool.intern(&stringPool, ".text"), ARAZU_OBJECT_SYMBOL_STATE_IN_SECTION, ARAZU_OBJECT_SYMBOL_VISIBILITY_GLOBAL, ARAZU_TRUE, 2, 0, stringPool.intern(&stringPool, "test.asm")) != ARAZU_TRUE)
    {
        fputs("Failed to create symbol test_function in .text\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_AddSymbol(context, section, symbol) != ARAZU_TRUE)
    {
        fputs("Failed to add symbol test_function in .text\n", stderr);

        Arazu_Object_Symbol_Destroy(context, symbol);
        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object_Symbol_Destroy(context, symbol);

    if (Arazu_Object_Symbol_Create(symbol, context, 5, stringPool.intern(&stringPool, "test_function.local_symbol"), stringPool.intern(&stringPool, ".text"), ARAZU_OBJECT_SYMBOL_STATE_IN_SECTION, ARAZU_OBJECT_SYMBOL_VISIBILITY_LOCAL, ARAZU_TRUE, 4, 0, stringPool.intern(&stringPool, "test.asm")) != ARAZU_TRUE)
    {
        fputs("Failed to create symbol test_function.local_symbol in .text\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_AddSymbol(context, section, symbol) != ARAZU_TRUE)
    {
        fputs("Failed to add symbol test_function.local_symbol in .text\n", stderr);

        Arazu_Object_Symbol_Destroy(context, symbol);
        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object_Symbol_Destroy(context, symbol);

    if (Arazu_Object_Relocation_Create(relocation, context, 0, 1, stringPool.intern(&stringPool, "number"), 4, ARAZU_OBJECT_RELOCATION_TYPE_ABSOLUTE, ARAZU_FALSE, ARAZU_TRUE, ARAZU_FALSE) != ARAZU_TRUE)
    {
        fputs("Failed to create relocation in .text\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_AddRelocation(context, section, relocation) != ARAZU_TRUE)
    {
        fputs("Failed to add relocation in .text\n", stderr);

        Arazu_Object_Relocation_Destroy(context, relocation);
        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object_Relocation_Destroy(context, relocation);

    if (Arazu_Object_AddSection(context, object, section) != ARAZU_TRUE)
    {
        fputs("Failed to add .text section to object\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }
    Arazu_Object_Section_Destroy(context, section);

    // .data

    if (Arazu_Object_Section_Create(section, context, stringPool.intern(&stringPool, ".data"), 8, ARAZU_OBJECT_SECTION_FLAGS_ALLOCATED, ARAZU_OBJECT_SECTION_TYPE_INITIALIZED) != ARAZU_TRUE)
    {
        fputs("Failed to create .data section\n", stderr);

        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_ReserveBufferSize(context, section, 23) != ARAZU_TRUE)
    {
        fputs("Failed to reserve 23 bytes for .data\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_ReserveSymbolCount(context, section, 2) != ARAZU_TRUE)
    {
        fputs("Failed to reserve 2 symbols for .data\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    (void)Arazu_Object_Section_PushByte(context, section, 'H');
    (void)Arazu_Object_Section_PushByte(context, section, 'e');
    (void)Arazu_Object_Section_PushByte(context, section, 'l');
    (void)Arazu_Object_Section_PushByte(context, section, 'l');
    (void)Arazu_Object_Section_PushByte(context, section, 'o');
    (void)Arazu_Object_Section_PushByte(context, section, ',');
    (void)Arazu_Object_Section_PushByte(context, section, ' ');
    (void)Arazu_Object_Section_PushByte(context, section, 'W');
    (void)Arazu_Object_Section_PushByte(context, section, 'o');
    (void)Arazu_Object_Section_PushByte(context, section, 'r');
    (void)Arazu_Object_Section_PushByte(context, section, 'l');
    (void)Arazu_Object_Section_PushByte(context, section, 'd');
    (void)Arazu_Object_Section_PushByte(context, section, '!');
    (void)Arazu_Object_Section_PushByte(context, section, '\n');
    (void)Arazu_Object_Section_PushByte(context, section, '\0');

    (void)Arazu_Object_Section_PushByte(context, section, 0x87);
    (void)Arazu_Object_Section_PushByte(context, section, 0xD6);
    (void)Arazu_Object_Section_PushByte(context, section, 0x12);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);
    (void)Arazu_Object_Section_PushByte(context, section, 0x00);

    if (Arazu_Object_Symbol_Create(symbol, context, 0, stringPool.intern(&stringPool, "msg"), stringPool.intern(&stringPool, ".data"), ARAZU_OBJECT_SYMBOL_STATE_IN_SECTION, ARAZU_OBJECT_SYMBOL_VISIBILITY_GLOBAL, ARAZU_TRUE, 10, 0, stringPool.intern(&stringPool, "test.asm")) != ARAZU_TRUE)
    {
        fputs("Failed to create symbol msg in .data\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_AddSymbol(context, section, symbol) != ARAZU_TRUE)
    {
        fputs("Failed to add symbol msg in .data\n", stderr);

        Arazu_Object_Symbol_Destroy(context, symbol);
        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object_Symbol_Destroy(context, symbol);

    if (Arazu_Object_Symbol_Create(symbol, context, 15, stringPool.intern(&stringPool, "number"), stringPool.intern(&stringPool, ".data"), ARAZU_OBJECT_SYMBOL_STATE_IN_SECTION, ARAZU_OBJECT_SYMBOL_VISIBILITY_GLOBAL, ARAZU_TRUE, 11, 0, stringPool.intern(&stringPool, "test.asm")) != ARAZU_TRUE)
    {
        fputs("Failed to create symbol number in .data\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Section_AddSymbol(context, section, symbol) != ARAZU_TRUE)
    {
        fputs("Failed to add symbol number in .data\n", stderr);

        Arazu_Object_Symbol_Destroy(context, symbol);
        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object_Symbol_Destroy(context, symbol);

    if (Arazu_Object_AddSection(context, object, section) != ARAZU_TRUE)
    {
        fputs("Failed to add .data section to object\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }
    Arazu_Object_Section_Destroy(context, section);


    Arazu_ELF_FileWriter fileWriter;
    if (createELFFileWriter(&fileWriter, filename) != ARAZU_TRUE)
    {
        fprintf(stderr, "Failed to open file %s\n", filename);

        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_ELF_WriteObject(context, object, &fileWriter) != ARAZU_TRUE)
    {
        fputs("Failed to write ELF file\n", stderr);

        destroyElfFileWriter(&fileWriter);
        Arazu_Object_Destroy(context, object);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    destroyElfFileWriter(&fileWriter);


    Arazu_Object_Destroy(context, object);

    Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), relocation);
    Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), symbol);
    Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);
    Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

    Arazu_Context_Destroy(context);
    stringPool.destroy(&stringPool);

    printf("This is just a placeholder. Do not try to this. This will may create a %s to test the library\n", filename);

    return 0;
}
