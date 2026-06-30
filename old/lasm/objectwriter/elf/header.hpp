#pragma once

#include <cstdint>

#include <toolchain.h>

namespace ELF
{
    namespace Bitness
    {
        constexpr uint8_t NONE   = 0;
        constexpr uint8_t BITS32 = 1;
        constexpr uint8_t BITS64 = 2;
    }

    namespace Endianness
    {
        constexpr uint8_t NONE = 0;
        constexpr uint8_t LSB2 = 1;
        constexpr uint8_t MSB2 = 2;
    }

    namespace Version
    {
        constexpr uint8_t NONE    = 0;
        constexpr uint8_t CURRENT = 1;
    }

    namespace ABI
    {
        constexpr uint8_t NONE       = 0;
        constexpr uint8_t HPUX       = 1;
        constexpr uint8_t NETBSD     = 2;
        constexpr uint8_t GNU        = 3;
        constexpr uint8_t SOLARIS    = 6;
        constexpr uint8_t AIX        = 7;
        constexpr uint8_t IRIX       = 8;
        constexpr uint8_t FREEBSD    = 9;
        constexpr uint8_t TRU64      = 10;
        constexpr uint8_t MODESTO    = 11;
        constexpr uint8_t OPENBSD    = 12;
        constexpr uint8_t ARM_AEABI  = 64;
        constexpr uint8_t ARM        = 97;
        constexpr uint8_t STANDALONE = 255;
    }

    namespace Type
    {
        constexpr uint16_t NONE        = 0;
        constexpr uint16_t RELOCATABLE = 1;
        constexpr uint16_t EXECUTABLE  = 2;
        constexpr uint16_t SHARED      = 3;
        constexpr uint16_t CORE        = 4;
    }

    namespace InstructionSet
    {
        constexpr uint16_t NONE         = 0;

        // AT&T
        constexpr uint16_t M32          = 1;

        // SUN
        constexpr uint16_t SPARC        = 2;
        constexpr uint16_t SPARC32PLUS  = 18;
        constexpr uint16_t SPARCV9      = 43;

        // Intel
        constexpr uint16_t I386         = 3;
        constexpr uint16_t IAMCU        = 6;
        constexpr uint16_t I860         = 7;
        constexpr uint16_t I960         = 19;
        constexpr uint16_t IA_64        = 50;
        constexpr uint16_t I8051        = 165;
        constexpr uint16_t L10M         = 180;
        constexpr uint16_t K10M         = 181;
        constexpr uint16_t INTELGT      = 205;

        // Motorola
        constexpr uint16_t M68K         = 4;
        constexpr uint16_t M88K         = 5;
        constexpr uint16_t RCE          = 39;
        constexpr uint16_t COLDFIRE     = 52;
        constexpr uint16_t M68HC12      = 53;
        constexpr uint16_t STARCORE     = 58;
        constexpr uint16_t M68HC16      = 69;
        constexpr uint16_t M68HC11      = 70;
        constexpr uint16_t M68HC08      = 71;
        constexpr uint16_t M68HC05      = 72;
        constexpr uint16_t XGATE        = 115;

        // MIPS
        constexpr uint16_t MIPS         = 8;
        constexpr uint16_t MIPS_RS3_LE  = 10;
        constexpr uint16_t MIPS_X       = 51;

        // IBM
        constexpr uint16_t S370         = 9;
        constexpr uint16_t S390         = 22;
        constexpr uint16_t SPU          = 23;

        // HPPA
        constexpr uint16_t PARISC       = 15;

        // Fujitsu
        constexpr uint16_t VPP500       = 17;
        constexpr uint16_t FR20         = 37;
        constexpr uint16_t MMA          = 54;
        constexpr uint16_t FR30         = 84;
        constexpr uint16_t F2MC16       = 104;

        // PowerPC
        constexpr uint16_t PPC          = 20;
        constexpr uint16_t PPC64        = 21;

        // NEC
        constexpr uint16_t V800         = 36;
        constexpr uint16_t V850         = 87;

        // TRW
        constexpr uint16_t RH32         = 38;

        // ARM
        constexpr uint16_t ARM          = 40;
        constexpr uint16_t AARCH64      = 183;

        // Digital
        constexpr uint16_t FAKE_ALPHA   = 41;
        constexpr uint16_t PDP10        = 64;
        constexpr uint16_t PDP11        = 65;

        // Hitachi
        constexpr uint16_t SH           = 42;
        constexpr uint16_t H8_300       = 46;
        constexpr uint16_t H8_300H      = 47;
        constexpr uint16_t H8S          = 48;
        constexpr uint16_t H8_500       = 49;

        // Siemens
        constexpr uint16_t TRICORE      = 44;
        constexpr uint16_t PCP          = 55;
        constexpr uint16_t FX66         = 66;

        // Argonaut
        constexpr uint16_t ARC          = 45;

        // Sony
        constexpr uint16_t NCPU         = 56;
        constexpr uint16_t PDSP         = 63;

        // Denso
        constexpr uint16_t NDR1         = 57;

        // Toyota
        constexpr uint16_t ME16         = 59;

        // STMicroelectronic
        constexpr uint16_t ST100        = 60;
        constexpr uint16_t ST9PLUS      = 67;
        constexpr uint16_t ST7          = 68;
        constexpr uint16_t ST19         = 74;
        constexpr uint16_t ST200        = 100;
        constexpr uint16_t MMDSP_PLUS   = 160;
        constexpr uint16_t STXP7X       = 166;
        constexpr uint16_t STM8         = 186;

        // Advanced Logic Corp.
        constexpr uint16_t TINYJ        = 61;

        // AMD
        constexpr uint16_t X86_64       = 62;
        constexpr uint16_t AMDGPU       = 224;

        // Silicon Graphics
        constexpr uint16_t SVX          = 73;

        // Digital VAX
        constexpr uint16_t VAX          = 75;

        // Axis Communications
        constexpr uint16_t CRIS         = 76;

        // Infineon Technologies
        constexpr uint16_t JAVELIN      = 77;
        constexpr uint16_t C166         = 116;
        constexpr uint16_t SLE9X        = 179;

        // Element 14
        constexpr uint16_t FIREPATH     = 78;

        // LSI Logic
        constexpr uint16_t ZSP          = 79;

        // Various (educational / research)
        constexpr uint16_t MMIX         = 80;   // Donald Knuth
        constexpr uint16_t HUANY        = 81;   // Harvard University
        constexpr uint16_t PRISM        = 82;   // SiTera

        // Atmel
        constexpr uint16_t AVR          = 83;
        constexpr uint16_t AVR32        = 185;

        // Mitsubishi
        constexpr uint16_t D10V         = 85;
        constexpr uint16_t D30V         = 86;
        constexpr uint16_t M32R         = 88;

        // Matsushita
        constexpr uint16_t MN10300      = 89;
        constexpr uint16_t MN10200      = 90;

        // picoJava
        constexpr uint16_t PJ           = 91;

        // OpenRISC
        constexpr uint16_t OPENRISC     = 92;

        // ARC International
        constexpr uint16_t ARC_COMPACT  = 93;

        // Tensilica
        constexpr uint16_t XTENSA       = 94;

        // Alphamosaic
        constexpr uint16_t VIDEOCORE    = 95;

        // Thompson Multimedia
        constexpr uint16_t TMM_GPP      = 96;

        // National Semiconductor
        constexpr uint16_t NS32K        = 97;
        constexpr uint16_t CR           = 103;
        constexpr uint16_t CRX          = 114;
        constexpr uint16_t CR16         = 177;

        // Tenor Network
        constexpr uint16_t TPC          = 98;

        // Trebia
        constexpr uint16_t SNP1K        = 99;

        // Ubicom
        constexpr uint16_t IP2K         = 101;

        // MAX
        constexpr uint16_t MAX          = 102;

        // Texas Instruments
        constexpr uint16_t MSP430       = 105;
        constexpr uint16_t TI_C6000     = 140;
        constexpr uint16_t TI_C2000     = 141;
        constexpr uint16_t TI_C5500     = 142;
        constexpr uint16_t TI_ARP32     = 143;
        constexpr uint16_t TI_PRU       = 144;

        // Analog Devices
        constexpr uint16_t BLACKFIN     = 106;
        constexpr uint16_t SHARC        = 133;

        // Seiko Epson
        constexpr uint16_t SE_C33       = 107;
        constexpr uint16_t SE_C17       = 139;

        // Sharp
        constexpr uint16_t SEP          = 108;

        // Arca
        constexpr uint16_t ARCA         = 109;

        // PKU-Unity / MPRC
        constexpr uint16_t UNICORE      = 110;

        // eXcess
        constexpr uint16_t EXCESS       = 111;

        // Icera Semiconductor
        constexpr uint16_t DXP          = 112;

        // Altera
        constexpr uint16_t ALTERA_NIOS2 = 113;

        // Renesas
        constexpr uint16_t M16C         = 117;
        constexpr uint16_t M32C         = 120;
        constexpr uint16_t R32C         = 162;
        constexpr uint16_t RX           = 173;
        constexpr uint16_t RL78         = 197;
        constexpr uint16_t R78KOR       = 199;

        // Microchip Technology
        constexpr uint16_t DSPIC30F     = 118;
        constexpr uint16_t MCHP_PIC     = 204;

        // Freescale
        constexpr uint16_t CE           = 119;
        constexpr uint16_t RS08         = 132;
        constexpr uint16_t ETPU         = 178;
        constexpr uint16_t F56800EX     = 200;

        // Altium
        constexpr uint16_t TSK3000      = 131;

        // Cyan Technology
        constexpr uint16_t ECOG2        = 134;
        constexpr uint16_t ECOG1X       = 168;
        constexpr uint16_t ECOG16       = 176;

        // Sunplus
        constexpr uint16_t SCORE7       = 135;

        // New Japan Radio (NJR)
        constexpr uint16_t DSP24        = 136;
        constexpr uint16_t XIMO16       = 170;

        // Broadcom
        constexpr uint16_t VIDEOCORE3   = 137;
        constexpr uint16_t VIDEOCORE5   = 198;

        // Lattice
        constexpr uint16_t LATTICEMICO32 = 138;

        // Cypress
        constexpr uint16_t CYPRESS_M8C  = 161;

        // NXP Semiconductors
        constexpr uint16_t TRIMEDIA     = 163;

        // Qualcomm
        constexpr uint16_t QDSP6        = 164;

        // Andes Technology
        constexpr uint16_t NDS32        = 167;

        // Dallas Semiconductor
        constexpr uint16_t MAXQ30       = 169;

        // M2000
        constexpr uint16_t MANIK        = 171;

        // Cray
        constexpr uint16_t CRAYNV2      = 172;

        // Imagination Technologies
        constexpr uint16_t METAG        = 174;

        // MCST
        constexpr uint16_t MCST_ELBRUS  = 175;

        // Tilera
        constexpr uint16_t TILE64       = 187;
        constexpr uint16_t TILEPRO      = 188;
        constexpr uint16_t TILEGX       = 191;

        // Xilinx
        constexpr uint16_t MICROBLAZE   = 189;

        // NVIDIA
        constexpr uint16_t CUDA         = 190;

        // CloudShield
        constexpr uint16_t CLOUDSHIELD  = 192;

        // KIPO-KAIST
        constexpr uint16_t COREA_1ST    = 193;
        constexpr uint16_t COREA_2ND    = 194;

        // Synopsys
        constexpr uint16_t ARCV2        = 195;

        // Open8
        constexpr uint16_t OPEN8        = 196;

        // Beyond
        constexpr uint16_t BA1          = 201;
        constexpr uint16_t BA2          = 202;

        // XMOS
        constexpr uint16_t XCORE        = 203;

        // KM211
        constexpr uint16_t KM32         = 210;
        constexpr uint16_t KMX32        = 211;
        constexpr uint16_t EMX16        = 212;
        constexpr uint16_t EMX8         = 213;
        constexpr uint16_t KVARC        = 214;

        // Paneve
        constexpr uint16_t CDP          = 215;

        // Cognitive
        constexpr uint16_t COGE         = 216;

        // Bluechip
        constexpr uint16_t COOL         = 217;

        // Nanoradio
        constexpr uint16_t NORC         = 218;

        // CSR
        constexpr uint16_t CSR_KALIMBA  = 219;

        // Zilog
        constexpr uint16_t Z80          = 220;

        // Controls and Data Services
        constexpr uint16_t VISIUM       = 221;

        // FTDI
        constexpr uint16_t FT32         = 222;

        // Moxie
        constexpr uint16_t MOXIE        = 223;

        // RISC-V
        constexpr uint16_t RISCV        = 243;

        // Linux BPF
        constexpr uint16_t BPF          = 247;

        // C-SKY
        constexpr uint16_t CSKY         = 252;

        // LoongArch
        constexpr uint16_t LOONGARCH    = 258;
    }

    PACKED_BEGIN

    struct Header32
    {
        // e_ident[16]
        uint8_t magic[4] = {0x7F, 'E', 'L', 'F'};
        uint8_t bitness;
        uint8_t endianness;
        uint8_t headerVersion;
        uint8_t abi;
        uint8_t _padding[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        uint16_t type;
        uint16_t instructionSet;
        uint32_t version;

        uint32_t programEntryPosition;
        uint32_t programHeaderTablePosition;
        uint32_t sectionHeaderTablePosition;

        uint32_t flags;
        uint16_t headerSize;

        uint16_t programHeaderTableEntrySize;
        uint16_t programHeaderTableEntryCount;

        uint16_t sectionHeaderTableEntrySize;
        uint16_t sectionHeaderTableEntryCount;
        uint16_t sectionNamesIndex;
    } __attribute__((packed));

    struct Header64
    {
        // e_ident[16]
        uint8_t magic[4] = {0x7F, 'E', 'L', 'F'};
        uint8_t bitness;
        uint8_t endianness;
        uint8_t headerVersion;
        uint8_t abi;
        uint8_t _padding[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        uint16_t type;
        uint16_t instructionSet;
        uint32_t version;

        uint64_t programEntryPosition;
        uint64_t programHeaderTablePosition;
        uint64_t sectionHeaderTablePosition;

        uint32_t flags;
        uint16_t headerSize;

        uint16_t programHeaderTableEntrySize;
        uint16_t programHeaderTableEntryCount;

        uint16_t sectionHeaderTableEntrySize;
        uint16_t sectionHeaderTableEntryCount;
        uint16_t sectionNamesIndex;
    } __attribute__((packed));

    PACKED_END


    // SECTIONS

    namespace SectionOffsets
    {
        constexpr uint32_t UNDEFINED = 0;
        constexpr uint32_t ABSOLUTE  = 0xFFF1;
        constexpr uint32_t COMMON    = 0xFFF2;
        constexpr uint32_t XINDEX    = 0xFFFF;
    }

    namespace SectionTypes
    {
        constexpr uint32_t UNUSED        = 0;
        constexpr uint32_t PROGBITS      = 1;
        constexpr uint32_t SYMTAB        = 2;
        constexpr uint32_t STRTAB        = 3;
        constexpr uint32_t RELA          = 4;
        constexpr uint32_t HASH          = 5;
        constexpr uint32_t DYNAMIC       = 6;
        constexpr uint32_t NOTE          = 7;
        constexpr uint32_t NOBITS        = 8;
        constexpr uint32_t REL           = 9;
        constexpr uint32_t SHLIB         = 10;
        constexpr uint32_t DYNSYM        = 11;
        constexpr uint32_t INIT_ARRAY    = 14;
        constexpr uint32_t FINI_ARRAY    = 15;
        constexpr uint32_t PREINIT_ARRAY = 16;
        constexpr uint32_t GROUP         = 17;
        constexpr uint32_t SYMTAB_SHNDX  = 18;
        constexpr uint32_t RELR          = 19;

        //constexpr uint32_t GNU_ATTRIBUTES = 0x6ffffff5;
        //constexpr uint32_t GNU_HASH       = 0x6ffffff6;
        //constexpr uint32_t GNU_LIBLIST    = 0x6ffffff7;
        //constexpr uint32_t CHECKSUM       = 0x6ffffff8;
        //constexpr uint32_t GNU_VERDEF     = 0x6ffffffd;
        //constexpr uint32_t GNU_VERNEED    = 0x6ffffffe;
        //constexpr uint32_t GNU_VERSYM     = 0x6fffffff;

        //constexpr uint32_t SUNW_COMDAT  = 0x6ffffffb;
        //constexpr uint32_t SUMW_SYMINFO = 0x6ffffffc;
    }

    namespace SectionFlags
    {
        constexpr uint32_t WRITE      = (1 << 0);
        constexpr uint32_t ALLOC      = (1 << 1);
        constexpr uint32_t EXECINSTR  = (1 << 2);
        constexpr uint32_t MERGE      = (1 << 4);
        constexpr uint32_t STRINGS    = (1 << 5);
        constexpr uint32_t INFO_LINK  = (1 << 6);
        constexpr uint32_t INFO_ORDER = (1 << 7);
        constexpr uint32_t GROUP      = (1 << 9);
        constexpr uint32_t TLS        = (1 << 10);
        constexpr uint32_t COMPRESSED = (1 << 11);

        //constexpr uint32_t GNU_RETAIN = (1 << 21);

        //constexpr uint32_t SUN_ORDERED = (1 << 30);
        //constexpr uint32_t SUN_EXCLUDE = (1U << 31);
    }

    PACKED_BEGIN

    struct SectionHeader32
    {
        uint32_t offsetInSectionNameStringTable;
        uint32_t type;
        uint32_t flags;
        uint32_t virtualAddress;
        uint32_t offset;
        uint32_t sectionSize;
        uint32_t linkIndex;
        uint32_t info;
        uint32_t addressAlignment;
        uint32_t entrySize;
    } __attribute__((packed));

    struct SectionHeader64
    {
        uint32_t offsetInSectionNameStringTable;
        uint32_t type;
        uint64_t flags;
        uint64_t virtualAddress;
        uint64_t offset;
        uint64_t sectionSize;
        uint32_t linkIndex;
        uint32_t info;
        uint64_t addressAlignment;
        uint64_t entrySize;
    } __attribute__((packed));

    PACKED_END


    namespace SectionCompressionType
    {
        constexpr uint32_t ZLIB = 1; // ZLIB/DEFLATE
        constexpr uint32_t ZSTD = 2; // Zstandard
    }

    PACKED_BEGIN

    struct SectionCompressionHeader32
    {
        uint32_t type;
        uint32_t uncompressedSize;
        uint32_t uncompressedAlignment;
    } __attribute__((packed));

    struct SectionCompressionHeader64
    {
        uint32_t type;
        uint32_t reserved;
        uint64_t uncompressedSize;
        uint64_t uncompressedAlignment;
    } __attribute__((packed));

    PACKED_END


    // SYMTAB

    PACKED_BEGIN

    struct SymbolEntry32
    {
        uint32_t indexInStringTable;
        uint32_t value;
        uint32_t size;
        uint8_t info;
        uint8_t other;
        uint16_t sectionIndex;
    } __attribute__((packed));

    struct SymbolEntry64
    {
        uint32_t indexInStringTable;
        uint8_t info;
        uint8_t other;
        uint16_t sectionIndex;
        uint64_t value;
        uint64_t size;
    } __attribute__((packed));

    PACKED_END

    namespace SymbolInfo
    {
        inline uint8_t GetBind(uint8_t val)
        {
            return static_cast<uint8_t>(val >> 4);
        }

        inline uint8_t GetType(uint8_t val)
        {
            return static_cast<uint8_t>(val & 0xF);
        }

        inline uint8_t GetInfo(uint8_t bind, uint8_t type)
        {
            return static_cast<uint8_t>((bind << 4) + (type & 0xF));
        }

        constexpr uint8_t BIND_LOCAL  = 0;
        constexpr uint8_t BIND_GLOBAL = 1;
        constexpr uint8_t BIND_WEAK   = 2;

        //constexpr uint8_t BIND_GNU_UNIQUE = 10;

        constexpr uint8_t TYPE_NONE    = 0;
        constexpr uint8_t TYPE_OBJECT  = 1;
        constexpr uint8_t TYPE_FUNC    = 2;
        constexpr uint8_t TYPE_SECTION = 3;
        constexpr uint8_t TYPE_FILE    = 4;
        constexpr uint8_t TYPE_COMMON  = 5;
        constexpr uint8_t TYPE_TLS     = 6;

        //constexpr uint8_t TYPE_GNU_IFUNC = 10;
    }

    namespace SymbolOther
    {
        inline uint8_t GetVisibility(uint8_t val)
        {
            return val & 0x3;
        }

        inline uint8_t GetOther(uint8_t visibility)
        {
            return visibility & 0x3;
        }

        constexpr uint8_t VISIBILITY_DEFAULT   = 0;
        constexpr uint8_t VISIBILITY_INTERNAL  = 1;
        constexpr uint8_t VISIBILITY_HIDDEN    = 2;
        constexpr uint8_t VISIBILITY_PROTECTED = 3;
    }

    PACKED_BEGIN

    struct SymbolInfoEntry
    {
        uint16_t boundto;
        uint16_t flags;
    } __attribute__((packed));

    PACKED_END

    namespace SymbolInfoBinding
    {
        constexpr uint16_t self   = 0xFFFF;
        constexpr uint16_t parent = 0xFFFE;
    }

    namespace SymbolInfoFlags
    {
        constexpr uint16_t direct   = (1 << 0);
        constexpr uint16_t passthru = (1 << 1);
        constexpr uint16_t copy     = (1 << 2);
        constexpr uint16_t lazeload = (1 << 3);
    }


    // RELOCATION

    namespace Relocation
    {
        PACKED_BEGIN

        struct Rel32
        {
            uint32_t offsetInSection;
            uint32_t info;
        } __attribute__((packed));

        struct Rel64
        {
            uint64_t offsetInSection;
            uint64_t info;
        } __attribute__((packed));

        struct Rela32
        {
            uint32_t offsetInSection;
            uint32_t info;
            int32_t addend;
        } __attribute__((packed));

        struct Rela64
        {
            uint64_t offsetInSection;
            uint64_t info;
            int64_t addend;
        } __attribute__((packed));

        PACKED_END

        inline uint32_t GetSym32(uint32_t val)
        {
            return static_cast<uint32_t>(val >> 8);
        }

        inline uint8_t GetType32(uint32_t val)
        {
            return static_cast<uint8_t>(val & 0xFF);
        }

        inline uint32_t GetInfo32(uint32_t sym, uint8_t type)
        {
            return static_cast<uint32_t>((sym << 8) + (type & 0xFF));
        }


        inline uint32_t GetSym64(uint64_t val)
        {
            return static_cast<uint32_t>(val >> 32);
        }

        inline uint32_t GetType64(uint64_t val)
        {
            return static_cast<uint32_t>(val & 0xFFFFFFFF);
        }

        inline uint64_t GetInfo64(uint32_t sym, uint32_t type)
        {
            return static_cast<uint64_t>((static_cast<uint64_t>(sym) << 32) + (type & 0xFF));
        }
    }


    // PROGRAM HEADERS

    PACKED_BEGIN

    struct ProgramSegmentHeader32
    {
        uint32_t type;
        uint32_t offset;
        uint32_t virtAddress;
        uint32_t physAddress;
        uint32_t sizeInFile;
        uint32_t sizeInMemory;
        uint32_t flags;
        uint32_t alignment;
    } __attribute__((packed));

    struct ProgramSegmentHeader64
    {
        uint32_t type;
        uint32_t flags;
        uint64_t offset;
        uint64_t virtAddress;
        uint64_t physAddress;
        uint64_t sizeInFile;
        uint64_t sizeInMemory;
        uint64_t alignment;
    } __attribute__((packed));

    PACKED_END

    constexpr uint16_t ProgramHeaderCountTooLarge = 0xFFFF;

    namespace ProgramSegmentType
    {
        constexpr uint32_t NONE    = 0;
        constexpr uint32_t LOAD    = 1;
        constexpr uint32_t DYNAMIC = 2;
        constexpr uint32_t INTERP  = 3;
        constexpr uint32_t NOTE    = 4;
        constexpr uint32_t SHLIB   = 5;
        constexpr uint32_t PHDR    = 6;
        constexpr uint32_t TLS     = 7;
        
        //constexpr uint32_t GNU_EH_FRAME = 0x6474E550;
        //constexpr uint32_t GNU_STACK    = 0x6474E551;
        //constexpr uint32_t GNU_RELRO    = 0x6474E552;
        //constexpr uint32_t GNU_PROPERTY = 0x6474E553;
        //constexpr uint32_t GNU_SFRAME   = 0x6474E554;

        //constexpr uint32_t SUNW_BSS   = 0x6FFFFFFA;
        //constexpr uint32_t SUNW_STACK = 0x6FFFFFFB;
    }

    namespace ProgramSegmentFlags
    {
        constexpr uint32_t EXECUTABLE = (1 << 0);
        constexpr uint32_t WRITABLE   = (1 << 1);
        constexpr uint32_t READABLE   = (1 << 2);
    }


    // NOTES

    PACKED_BEGIN

    struct NoteHeader
    {
        uint32_t lengthOfName;
        uint32_t lengthOfDescriptor;
        uint32_t type;
    } __attribute__((packed));

    PACKED_END


    // SPECIFIC

    namespace Motorola
    {
        // TODO: Check
        constexpr uint32_t FLAGS_CPU32 = 0x00800000;
        constexpr uint32_t FLAGS_FPU   = 0x00010000;

        namespace Relocations
        {
            constexpr uint8_t R68K_NONE         = 0;
            constexpr uint8_t R68K_32           = 1;
            constexpr uint8_t R68K_16           = 2;
            constexpr uint8_t R68K_8            = 3;
            constexpr uint8_t R68K_PC32         = 4;
            constexpr uint8_t R68K_PC16         = 5;
            constexpr uint8_t R68K_PC8          = 6;
            constexpr uint8_t R68K_GOT32        = 7;
            constexpr uint8_t R68K_GOT16        = 8;
            constexpr uint8_t R68K_GOT8         = 9;
            constexpr uint8_t R68K_GOT32O       = 10;
            constexpr uint8_t R68K_GOT16O       = 11;
            constexpr uint8_t R68K_GOT8O        = 12;
            constexpr uint8_t R68K_PLT32        = 13;
            constexpr uint8_t R68K_PLT16        = 14;
            constexpr uint8_t R68K_PLT8         = 15;
            constexpr uint8_t R68K_PLT32O       = 16;
            constexpr uint8_t R68K_PLT16O       = 17;
            constexpr uint8_t R68K_PLT8O        = 18;
            constexpr uint8_t R68K_COPY         = 19;
            constexpr uint8_t R68K_GLOB_DAT     = 20;
            constexpr uint8_t R68K_JMP_SLOT     = 21;
            constexpr uint8_t R68K_RELATIVE     = 22;
            constexpr uint8_t R68K_TLS_GD32     = 25;
            constexpr uint8_t R68K_TLS_GD16     = 26;
            constexpr uint8_t R68K_TLS_GD8      = 27;
            constexpr uint8_t R68K_TLS_LDM32    = 28;
            constexpr uint8_t R68K_TLS_LDM16    = 29;
            constexpr uint8_t R68K_TLS_LDM8     = 30;
            constexpr uint8_t R68K_TLS_LDO32    = 31;
            constexpr uint8_t R68K_TLS_LDO16    = 32;
            constexpr uint8_t R68K_TLS_LDO8     = 33;
            constexpr uint8_t R68K_TLS_IE32     = 34;
            constexpr uint8_t R68K_TLS_IE16     = 35;
            constexpr uint8_t R68K_TLS_IE8      = 36;
            constexpr uint8_t R68K_TLS_LE32     = 37;
            constexpr uint8_t R68K_TLS_LE16     = 38;
            constexpr uint8_t R68K_TLS_LE8      = 39;
            constexpr uint8_t R68K_TLS_DTPMOD32 = 40;
            constexpr uint8_t R68K_TLS_DTPREL32 = 41;
            constexpr uint8_t R68K_TLS_TPREL32  = 42;
        }
    }

    namespace Intel
    {
        namespace Relocations
        {
            constexpr uint8_t R386_NONE          = 0;
            constexpr uint8_t R386_32            = 1;
            constexpr uint8_t R386_PC32          = 2;
            constexpr uint8_t R386_GOT32         = 3;
            constexpr uint8_t R386_PLT32         = 4;
            constexpr uint8_t R386_COPY          = 5;
            constexpr uint8_t R386_GLOB_DAT      = 6;
            constexpr uint8_t R386_JMP_SLOT      = 7;
            constexpr uint8_t R386_RELATIVE      = 8;
            constexpr uint8_t R386_GOTOFF        = 9;
            constexpr uint8_t R386_GOTPC         = 10;
            constexpr uint8_t R386_32PLT         = 11;
            constexpr uint8_t R386_TLS_TPOFF     = 14;
            constexpr uint8_t R386_TLS_IE        = 15;
            constexpr uint8_t R386_TLS_GOTIE     = 16;
            constexpr uint8_t R386_TLS_LE        = 17;
            constexpr uint8_t R386_TLS_GD        = 18;
            constexpr uint8_t R386_TLS_LDM       = 19;
            constexpr uint8_t R386_16            = 20;
            constexpr uint8_t R386_PC16          = 21;
            constexpr uint8_t R386_8             = 22;
            constexpr uint8_t R386_PC8           = 23;
            constexpr uint8_t R386_TLS_GD_32     = 24;
            constexpr uint8_t R386_TLS_GD_PUSH   = 25;
            constexpr uint8_t R386_TLS_GD_CALL   = 26;
            constexpr uint8_t R386_TLS_GD_POP    = 27;
            constexpr uint8_t R386_TLS_LDM_32    = 28;
            constexpr uint8_t R386_TLS_LDM_PUSH  = 29;
            constexpr uint8_t R386_TLS_LDM_CALL  = 30;
            constexpr uint8_t R386_TLS_LDM_POP   = 31;
            constexpr uint8_t R386_TLS_LDO_32    = 32;
            constexpr uint8_t R386_TLS_IE_32     = 33;
            constexpr uint8_t R386_TLS_LE_32     = 34;
            constexpr uint8_t R386_TLS_DTPMOD32  = 35;
            constexpr uint8_t R386_TLS_DTPOFF32  = 36;
            constexpr uint8_t R386_TLS_TPOFF32   = 37;
            constexpr uint8_t R386_SIZE32        = 38;
            constexpr uint8_t R386_TLS_GOTDESC   = 39;
            constexpr uint8_t R386_TLS_DESC_CALL = 40;
            constexpr uint8_t R386_TLS_DESC      = 41;
            constexpr uint8_t R386_IRELATIVE     = 42;
            constexpr uint8_t R386_GOT32X        = 43;
        }
    }

    namespace AMD
    {
        constexpr uint32_t SECTIONTYPE_UNWIND = 0x70000001;

        namespace Relocations
        {
            constexpr uint8_t R64_NONE       = 0;
            constexpr uint8_t R64_64         = 1;
            constexpr uint8_t R64_PC32       = 2;
            constexpr uint8_t R64_GOT32      = 3;
            constexpr uint8_t R64_PLT32      = 4;
            constexpr uint8_t R64_COPY       = 5;
            constexpr uint8_t R64_GLOB_DAT   = 6;
            constexpr uint8_t R64_JMP_SLOT   = 7;
            constexpr uint8_t R64_RELATIVE   = 8;
            constexpr uint8_t R64_GOTPCREL   = 9;
            constexpr uint8_t R64_32         = 10;
            constexpr uint8_t R64_32S        = 11;
            constexpr uint8_t R64_16         = 12;
            constexpr uint8_t R64_PC16       = 13;
            constexpr uint8_t R64_8          = 14;
            constexpr uint8_t R64_PC8        = 15;
            constexpr uint8_t R64_DTPMOD64   = 16;
            constexpr uint8_t R64_DTPOFF64   = 17;
            constexpr uint8_t R64_TPOFF64    = 18;
            constexpr uint8_t R64_TLSGD      = 19;
            constexpr uint8_t R64_TLSLD      = 20;
            constexpr uint8_t R64_DTPOFF32   = 21;
            constexpr uint8_t R64_GOTTPOFF   = 22;
            constexpr uint8_t R64_TPOFF32    = 23;
            constexpr uint8_t R64_PC64       = 24;
            constexpr uint8_t R64_GOTOFF64   = 25;
            constexpr uint8_t R64_GOTPC32    = 26;
            constexpr uint8_t R64_GOT64      = 27;
            constexpr uint8_t R64_GOTPCREL64 = 28;
            constexpr uint8_t R64_GOTPC64    = 29;
            constexpr uint8_t R64_GOTPLT64   = 30;
            constexpr uint8_t R64_PLTOFF64   = 31;
            constexpr uint8_t R64_SIZE32     = 32;
            constexpr uint8_t R64_SIZE64     = 33;
            constexpr uint8_t R64_GOTPC32_TLSDESC = 34;
            constexpr uint8_t R64_TLSDESC_CALL    = 35;
            constexpr uint8_t R64_TLSDESC         = 36;
            constexpr uint8_t R64_IRELATIVE       = 37;
            constexpr uint8_t R64_RELATIVE64      = 38;
            //constexpr uint8_t R64_PC32_BND        = 39;
            //constexpr uint8_t R64_PLT32_BND       = 40;
            constexpr uint8_t R64_GOTPCRELX       = 41;
            constexpr uint8_t R64_REX_GOTPCRELX   = 42;
        }
    }

}
