#ifndef ARAZU_ELF_WRITER_H
#define ARAZU_ELF_WRITER_H

#include <arazu/core/types.h>
#include <arazu/core/object/object.h>

typedef struct Arazu_ELF_FileWriter
{
    /** Write size bytes of data to the file without changing it in any way (raw) */
    Arazu_Bool (*write)(struct Arazu_ELF_FileWriter* writer, const Arazu_u8* data, Arazu_Size size);

    void* userdata;
} Arazu_ELF_FileWriter;

/** Write an object to an ELF file */
ARAZU_DETAIL_API Arazu_Bool Arazu_ELF_WriteObject(const Arazu_Object* object, Arazu_ELF_FileWriter* fileWriter);

#endif
