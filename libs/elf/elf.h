#ifndef ARAZU_LELF_ELF_H
#define ARAZU_LELF_ELF_H

#ifdef __cplusplus
extern "C" {
#endif

/*
Elf32_Addr      = Arazu_u32
Elf32_Half      = Arazu_u16
Elf32_Off       = Arazu_u32
Elf32_Sword     = Arazu_i32
Elf32_Word      = Arazu_u32
Elf32_Lword     = Arazu_u64

Elf32_Hashelt   = Arazu_u32
Elf32_Size      = Arazu_u32
Elf32_Ssize     = Arazu_i32

Elf64_Addr      = Arazu_u64
Elf64_Half      = Arazu_u16
Elf64_Off       = Arazu_u64
Elf64_Sword     = Arazu_i32
Elf64_Sxword    = Arazu_i64
Elf64_Word      = Arazu_u32
Elf64_Lword     = Arazu_u64
Elf64_Xword     = Arazu_u64

Elf64_Hashelt   = Arazu_u32
Elf64_Size      = Arazu_u64
Elf64_Ssize     = Arazu_i64
*/

/*
    Header:
        Arazu_u8 ident[16]:
            Arazu_u8 magic[4];
            Arazu_u8 class;
            Arazu_u8 endianness;
            Arazu_u8 fileVersion;
            Arazu_u8 osabi;
            Arazu_u8 abiVersion;
            Arazu_u8 padding[7];

        Arazu_u16 type;
        Arazu_u16 machine;
        Arazu_u32 version;
        
        # 0 if none
        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) entry;

        # 0 if none
        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) programHeaderTableOffset;
        
        # 0 if none
        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) sectionHeaderTableOffset;

        Arazu_u32 flags;

        Arazu_u16 headerSize;
        Arazu_u16 programHeaderSize;
        Arazu_u16 programHeaderCount;
        Arazu_u16 sectionHeaderSize;

        # if greater than or egual to 0xFF00, it has the value 0
        # The actual index is in the size field of the section header at index 0
        Arazu_u16 sectionHeaderCount;

        # if greater than or egual to 0xFF00, it has the value 0xFFFF
        # The actual index is in the link field of the section header at index 0
        Arazu_u16 sectionNameStringTableIndex;

    Section header:
        # index in section header string table
        Arazu_u32 name;

        Arazu_u32 type;
        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) flags;

        # address of section's first byte if loaded into memory of a process, 0 if not
        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) address;

        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) offset;
        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) size;

        Arazu_u32 link;
        Arazu_u32 info;

        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) alignment;
        Arazu_u32 (32-bit) / Arazu_u64 (64-bit) entrySize;
*/

#define ELF_HEADER_SIZE 64

#define ELF_HEADER_SIZE_64 64
#define ELF_HEADER_SIZE_32 52

/* ident */
#define ELF_HEADER_IDENT_MAGIC_0 ((Arazu_u8)0x7F)
#define ELF_HEADER_IDENT_MAGIC_1 ((Arazu_u8)'E')
#define ELF_HEADER_IDENT_MAGIC_2 ((Arazu_u8)'L')
#define ELF_HEADER_IDENT_MAGIC_3 ((Arazu_u8)'F')

#define ELF_HEADER_IDENT_CLASS_NONE ((Arazu_u8)0)
#define ELF_HEADER_IDENT_CLASS_32   ((Arazu_u8)1)
#define ELF_HEADER_IDENT_CLASS_64   ((Arazu_u8)2)

#define ELF_HEADER_IDENT_ENDIANNESS_NONE   ((Arazu_u8)0)
#define ELF_HEADER_IDENT_ENDIANNESS_LITTLE ((Arazu_u8)1)
#define ELF_HEADER_IDENT_ENDIANNESS_BIG    ((Arazu_u8)2)

#define ELF_HEADER_IDENT_VERSION ((Arazu_u8)1)

#define ELF_HEADER_IDENT_OSABI_NONE    ((Arazu_u8)0)
#define ELF_HEADER_IDENT_OSABI_HPUX    ((Arazu_u8)1)
#define ELF_HEADER_IDENT_OSABI_NETBSD  ((Arazu_u8)2)
#define ELF_HEADER_IDENT_OSABI_LINUX   ((Arazu_u8)3)
#define ELF_HEADER_IDENT_OSABI_SOLARIS ((Arazu_u8)6)
#define ELF_HEADER_IDENT_OSABI_AIX     ((Arazu_u8)7)
#define ELF_HEADER_IDENT_OSABI_IRIX    ((Arazu_u8)8)
#define ELF_HEADER_IDENT_OSABI_FREEBSD ((Arazu_u8)9)
#define ELF_HEADER_IDENT_OSABI_TRU64   ((Arazu_u8)10)
#define ELF_HEADER_IDENT_OSABI_MODESTO ((Arazu_u8)11)
#define ELF_HEADER_IDENT_OSABI_OPENBSD ((Arazu_u8)12)
#define ELF_HEADER_IDENT_OSABI_OPENVMS ((Arazu_u8)13)
#define ELF_HEADER_IDENT_OSABI_NSK     ((Arazu_u8)14)

#define ELF_HEADER_IDENT_ABIVERSION_UNSPECIFIED ((Arazu_u8)0)

/* type */
#define ELF_HEADER_TYPE_NONE        ((Arazu_u16)0)
#define ELF_HEADER_TYPE_RELOCATABLE ((Arazu_u16)1)
#define ELF_HEADER_TYPE_EXECUTABLE  ((Arazu_u16)2)
#define ELF_HEADER_TYPE_DYNAMIC     ((Arazu_u16)3)
#define ELF_HEADER_TYPE_CORE        ((Arazu_u16)4)

/* machine */
#define ELF_HEADER_MACHINE_NONE   ((Arazu_u16)0)
/* TODO: Add values */
#define ELF_HEADER_MACHINE_386    ((Arazu_u16)3)   /* Intel 80386 */
#define ELF_HEADER_MACHINE_860    ((Arazu_u16)7)   /* Intel 80860 */
#define ELF_HEADER_MACHINE_960    ((Arazu_u16)19)  /* Intel 80960 */
#define ELF_HEADER_MACHINE_IA_64  ((Arazu_u16)50)  /* Intel IA-64 */
#define ELF_HEADER_MACHINE_X86_64 ((Arazu_u16)62)  /* AMD x86-64 */

/* version */
#define ELF_HEADER_VERSION_NONE    ((Arazu_u32)0)
#define ELF_HEADER_VERSION_CURRENT ((Arazu_u32)1)



#define ELF_SECTIONHEADER_SIZE 64

#define ELF_SECTIONHEADER_SIZE_64 64
#define ELF_SECTIONHEADER_SIZE_32 40

/* type */
#define ELF_SECTIONHEADER_TYPE_NULL          ((Arazu_u32)0)

#define ELF_SECTIONHEADER_TYPE_PROGBITS      ((Arazu_u32)1)
#define ELF_SECTIONHEADER_TYPE_NOBITS        ((Arazu_u32)8)

#define ELF_SECTIONHEADER_TYPE_SYMTAB        ((Arazu_u32)2)
#define ELF_SECTIONHEADER_TYPE_DYNSYM        ((Arazu_u32)11)
#define ELF_SECTIONHEADER_TYPE_SYMTAB_SHNDX  ((Arazu_u32)18)

#define ELF_SECTIONHEADER_TYPE_STRTAB        ((Arazu_u32)3)

#define ELF_SECTIONHEADER_TYPE_RELA          ((Arazu_u32)4)
#define ELF_SECTIONHEADER_TYPE_REL           ((Arazu_u32)9)

#define ELF_SECTIONHEADER_TYPE_NOTE          ((Arazu_u32)7)
#define ELF_SECTIONHEADER_TYPE_DYNAMIC       ((Arazu_u32)6)

#define ELF_SECTIONHEADER_TYPE_HASH          ((Arazu_u32)5)

#define ELF_SECTIONHEADER_TYPE_INIT_ARRAY    ((Arazu_u32)14)
#define ELF_SECTIONHEADER_TYPE_FINI_ARRAY    ((Arazu_u32)15)
#define ELF_SECTIONHEADER_TYPE_PREINIT_ARRAY ((Arazu_u32)16)

#define ELF_SECTIONHEADER_TYPE_SHLIB         ((Arazu_u32)10)
#define ELF_SECTIONHEADER_TYPE_GROUP         ((Arazu_u32)17)

/* flags */
#define ELF_SECTIONHEADER_FLAGS_WRITE      ((Arazu_u32)0x1)
#define ELF_SECTIONHEADER_FLAGS_ALLOC      ((Arazu_u32)0x2)
#define ELF_SECTIONHEADER_FLAGS_EXECINSTR  ((Arazu_u32)0x4)
#define ELF_SECTIONHEADER_FLAGS_MERGE      ((Arazu_u32)0x10)
#define ELF_SECTIONHEADER_FLAGS_STRINGS    ((Arazu_u32)0x20)
#define ELF_SECTIONHEADER_FLAGS_INFO_LINK  ((Arazu_u32)0x40)
#define ELF_SECTIONHEADER_FLAGS_LINK_ORDER ((Arazu_u32)0x80)
#define ELF_SECTIONHEADER_FLAGS_GROUP      ((Arazu_u32)0x200)
#define ELF_SECTIONHEADER_FLAGS_TLS        ((Arazu_u32)0x400)

/* link & info

    DYNAMIC:
        link: section header index of the string table used
        info: 0

    HASH:
        link: section header index of the associated string table
        info: 0

    REL & RELA:
        link: section header index of the associated symbol table
        info: section header index of the section to which the relocations apply

    SYMTAB & DYNSYM:
        link: section header index of the associated string table
        info: one greater than the symbol table index of the last local symbol

    GROUP:
        link: section header index of the associated symbol table
        info: symbol table index of an enrty in the associated symbol table

    SYMTAB_SHNDX:
        link: section header index of the associated symbol table section
        info: 0

*/

#ifdef __cplusplus
}
#endif

#endif
