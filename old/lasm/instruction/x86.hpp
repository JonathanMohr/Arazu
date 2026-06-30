#pragma once

#include <cstdint>
#include <unordered_map>
#include <string_view>
#include <Architecture.hpp>

namespace Assembler
{
    namespace x86
    {
        constexpr inline uint8_t getRex(bool w, bool r, bool x, bool b)
        {
            uint8_t rex = 0b01000000;
            if (w) rex |= 0b00001000;
            if (r) rex |= 0b00000100;
            if (x) rex |= 0b00000010;
            if (b) rex |= 0b00000001;
            return rex;
        }

        enum class Mod : uint8_t {
            INDIRECT        = 0b00,
            INDIRECT_DISP8  = 0b01,
            INDIRECT_DISP32 = 0b10,
            REGISTER        = 0b11
        };
        constexpr uint8_t modRMSIB = 0b100;
        constexpr uint8_t modRMDisp = 0b101;
        inline uint8_t getModRM(Mod mod, uint8_t reg, uint8_t rm)
        {
            uint8_t modrm = static_cast<uint8_t>((static_cast<uint8_t>(mod) << 6) | (reg << 3) | rm);
            return modrm;
        }

        enum class Scale : uint8_t {
            x1 = 0,
            x2 = 1,
            x4 = 2,
            x8 = 3
        };
        constexpr uint8_t SIB_NoIndex = 0b100;
        inline uint8_t getSIB(Scale scale, uint8_t index, uint8_t base)
        {
            uint8_t sib = static_cast<uint8_t>((static_cast<uint8_t>(scale) << 6) | (index << 3) | base);
            return sib;
        }

        enum class Instruction : uint8_t
        {
            // ASCII
            AAA, AAD, AAM, AAS,

            // CONTROL
            NOP,
            HLT,

            JMP, JE, JNE,
            JG, JGE, JL, JLE,
            JA, JAE, JB, JBE,
            JO, JNO, JS, JNS, JP, JNP, JC, JNC,

            CALL, RET,

            // INTERRUPT
            INT, INT3, INTO, INT1,
            IRET, IRETQ, IRETD,
            SYSCALL, SYSRET,
            SYSENTER, SYSEXIT,

            // FLAGS
            CLC, STC, CMC,
            CLD, STD,
            CLI, STI,
            LAHF, SAHF,

            // STACK
            PUSH, POP,
            PUSHA, POPA,
            PUSHAD, POPAD,
            PUSHF, POPF,
            PUSHFD, POPFD,
            PUSHFQ, POPFQ,

            // DATA
            MOV,

            // ALU
            ADD, ADC, SUB, SBB, CMP, TEST, AND, OR, XOR,

            ADCX, ADOX,
            
            MUL, IMUL, DIV, IDIV,

            SHL, SHR, SAL, SAR,
            ROL, ROR, RCL, RCR,

            NOT, NEG, INC, DEC,
        };

        static const std::unordered_map<std::string_view, Instruction> instructionMap = {
            // ASCII
            {"aaa", Instruction::AAA}, {"aad", Instruction::AAD},
            {"aam", Instruction::AAM}, {"aas", Instruction::AAS},

            // CONTROL
            {"nop", Instruction::NOP}, {"hlt", Instruction::HLT},

            {"jmp", Instruction::JMP}, {"je", Instruction::JE}, {"jz", Instruction::JE},
            {"jne", Instruction::JNE}, {"jnz", Instruction::JNE},

            {"jg", Instruction::JG}, {"jnle", Instruction::JG}, {"jge", Instruction::JGE}, {"jnl", Instruction::JGE},
            {"jl", Instruction::JL}, {"jnge", Instruction::JL}, {"jle", Instruction::JLE}, {"jng", Instruction::JLE},

            {"ja", Instruction::JA}, {"jnbe", Instruction::JA}, {"jae", Instruction::JAE}, {"jnb", Instruction::JAE},
            {"jb", Instruction::JB}, {"jnae", Instruction::JB}, {"jbe", Instruction::JBE}, {"jna", Instruction::JBE},

            {"jo", Instruction::JO}, {"jno", Instruction::JNO}, {"js", Instruction::JS}, {"jns", Instruction::JNS},
            {"jp", Instruction::JP}, {"jpe", Instruction::JP}, {"jnp", Instruction::JNP}, {"jpo", Instruction::JNP},
            {"jc", Instruction::JC}, {"jnc", Instruction::JNC},

            {"call", Instruction::CALL}, {"ret", Instruction::RET},

            // INTERRUPT
            {"int", Instruction::INT}, {"int3", Instruction::INT3},
            {"into", Instruction::INTO},
            {"int1", Instruction::INT1}, {"icebp", Instruction::INT1},

            {"iret", Instruction::IRET}, {"iretq", Instruction::IRETQ}, {"iretd", Instruction::IRETD},
            {"syscall", Instruction::SYSCALL}, {"sysret", x86::Instruction::SYSRET},
            {"sysenter", x86::Instruction::SYSENTER}, {"sysexit", x86::Instruction::SYSEXIT},

            // FLAGS
            {"clc", Instruction::CLC}, {"stc", Instruction::STC}, {"cmc", Instruction::CMC},
            {"cld", Instruction::CLD}, {"std", Instruction::STD},
            {"cli", Instruction::CLI}, {"sti", Instruction::STI},
            {"lahf", Instruction::LAHF}, {"sahf", Instruction::SAHF},

            // STACK
            {"push", Instruction::PUSH}, {"pop", Instruction::POP},
            {"pusha", Instruction::PUSHA}, {"popa", Instruction::POPA},
            {"pushad", Instruction::PUSHAD}, {"popad", Instruction::POPAD},
            {"pushf", Instruction::PUSHF}, {"popf", Instruction::POPF},
            {"pushfd", Instruction::PUSHFD}, {"popfd", Instruction::POPFD},
            {"pushfq", Instruction::PUSHFQ}, {"popfq", Instruction::POPFQ},

            // DATA
            {"mov", Instruction::MOV},

            // ALU
            {"add", Instruction::ADD}, {"adc", Instruction::ADC},
            {"sub", Instruction::SUB}, {"sbb", Instruction::SBB},
            {"cmp", Instruction::CMP}, {"test", Instruction::TEST},
            {"and", Instruction::AND}, {"or", Instruction::OR},
            {"xor", Instruction::XOR},

            {"adcx", Instruction::ADCX}, {"adox", Instruction::ADOX},

            {"mul", Instruction::MUL}, {"imul", Instruction::IMUL},
            {"div", Instruction::DIV}, {"idiv", Instruction::IDIV},

            {"shl", Instruction::SHL}, {"shr", Instruction::SHR},
            {"sal", Instruction::SAL}, {"sar", Instruction::SAR},
            {"rol", Instruction::ROL}, {"ror", Instruction::ROR},
            {"rcl", Instruction::RCL}, {"rcr", Instruction::RCR},

            {"not", Instruction::NOT}, {"neg", Instruction::NEG},
            {"inc", Instruction::INC}, {"dec", Instruction::DEC},
        };

        enum Register : uint8_t
        {
            AL, BL, CL, DL,
            AH, BH, CH, DH,
            SPL, BPL, SIL, DIL,
            R8B, R9B, R10B, R11B,
            R12B, R13B, R14B, R15B,

            AX, BX, CX, DX,
            SP, BP, SI, DI,
            ES, CS, SS, DS, FS, GS,
            FLAGS,
            R8W, R9W, R10W, R11W,
            R12W, R13W, R14W, R15W,

            EAX, EBX, ECX, EDX,
            ESP, EBP, ESI, EDI,
            R8D, R9D, R10D, R11D,
            R12D, R13D, R14D, R15D,
            CR0, /*CR1,*/ CR2, CR3, CR4, CR5, CR6, CR7,
            DR0, DR1, DR2, DR3, /*DR4, DR5,*/ DR6, DR7,
            ST0, ST1, ST2, ST3, ST4, ST5, ST6, ST7,
            EFLAGS,

            MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7,

            RAX, RBX, RCX, RDX,
            RSP, RBP, RSI, RDI,
            R8, R9, R10, R11,
            R12, R13, R14, R15,
            CR8, CR9, CR10, CR11, CR12, CR13, CR14, CR15,
            DR8, DR9, DR10, DR11, DR12, DR13, DR14, DR15,
            RFLAGS,

            XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,
            XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15,

            YMM0, YMM1, YMM2, YMM3, YMM4, YMM5, YMM6, YMM7,
            YMM8, YMM9, YMM10, YMM11, YMM12, YMM13, YMM14, YMM15,

            ZMM0, ZMM1, ZMM2, ZMM3, ZMM4, ZMM5, ZMM6, ZMM7,
            ZMM8, ZMM9, ZMM10, ZMM11, ZMM12, ZMM13, ZMM14, ZMM15,
            ZMM16, ZMM17, ZMM18, ZMM19, ZMM20, ZMM21, ZMM22, ZMM23,
            ZMM24, ZMM25, ZMM26, ZMM27, ZMM28, ZMM29, ZMM30, ZMM31,

            K0, K1, K2, K3, K4, K5, K6, K7,

            MXCSR,

            BND0, BND1, BND2, BND3,
            BNDCFGU, BNDCFGS,

            XCR0, PKRU
        };

        // tuple:
        //   uint8_t: register index
        //   bool:    use REX to address
        //   bool:    set REX byte
        std::tuple<uint8_t, bool, bool> getRegInfo(uint64_t reg, uint64_t line, uint64_t column, const char* file);

        uint64_t getRegSize(uint64_t reg, BitMode mode, uint64_t line, uint64_t column, const char* file);
        bool isRegGPR(uint64_t reg);
        bool isRegSegment(uint64_t reg);

        static const std::unordered_map<std::string_view, Register> registerMap = {
            {"al", Register::AL}, {"bl", Register::BL}, {"cl", Register::CL}, {"dl", Register::DL},
            {"ah", Register::AH}, {"bh", Register::BH}, {"ch", Register::CH}, {"dh", Register::DH},
            {"spl", Register::SPL}, {"bpl", Register::BPL}, {"sil", Register::SIL}, {"dil", Register::DIL},
            {"r8b", Register::R8B}, {"r9b", Register::R9B}, {"r10b", Register::R10B}, {"r11b", Register::R11B},
            {"r12b", Register::R12B}, {"r13b", Register::R13B}, {"r14b", Register::R14B}, {"r15b", Register::R15B},

            {"ax", Register::AX}, {"bx", Register::BX}, {"cx", Register::CX}, {"dx", Register::DX},
            {"sp", Register::SP}, {"bp", Register::BP}, {"si", Register::SI}, {"di", Register::DI},
            {"es", Register::ES}, {"cs", Register::CS}, {"ss", Register::SS}, {"ds", Register::DS}, {"fs", Register::FS}, {"gs", Register::GS},
            {"flags", Register::FLAGS},
            {"r8w", Register::R8W}, {"r9w", Register::R9W}, {"r10w", Register::R10W}, {"r11w", Register::R11W},
            {"r12w", Register::R12W}, {"r13w", Register::R13W}, {"r14w", Register::R14W}, {"r15w", Register::R15W},

            // 32-bit registers
            {"eax", Register::EAX}, {"ebx", Register::EBX}, {"ecx", Register::ECX}, {"edx", Register::EDX},
            {"esp", Register::ESP}, {"ebp", Register::EBP}, {"esi", Register::ESI}, {"edi", Register::EDI},
            {"r8d", Register::R8D}, {"r9d", Register::R9D}, {"r10d", Register::R10D}, {"r11d", Register::R11D},
            {"r12d", Register::R12D}, {"r13d", Register::R13D}, {"r14d", Register::R14D}, {"r15d", Register::R15D},

            // Control registers
            {"cr0", Register::CR0}, /*{"cr1": Register::CR1,}*/  // CR1 is not used in x86-64
            {"cr2", Register::CR2}, 
            {"cr3", Register::CR3},
            {"cr4", Register::CR4},
            {"cr5", Register::CR5},
            {"cr6", Register::CR6},
            {"cr7", Register::CR7},

            // Debug registers
            {"dr0", Register::DR0},
            {"dr1", Register::DR1},
            {"dr2", Register::DR2},
            {"dr3", Register::DR3},
            /*{"dr4": Register::DR4, "dr5": Register::DR5,}*/  // DR4 and DR5 are not used in x86-64
            {"dr6", Register::DR6},
            {"dr7", Register::DR7},
            {"dr8", Register::DR8},
            {"dr9", Register::DR9},
            {"dr10", Register::DR10},
            {"dr11", Register::DR11},
            {"dr12", Register::DR12},
            {"dr13", Register::DR13},
            {"dr14", Register::DR14},
            {"dr15", Register::DR15},

            // Flags register
            {"eflags", Register::EFLAGS},

            // 64-bit registers
            {"rax", Register::RAX}, {"rbx", Register::RBX}, {"rcx", Register::RCX}, {"rdx", Register::RDX},
            {"rsp", Register::RSP}, {"rbp", Register::RBP}, {"rsi", Register::RSI}, {"rdi", Register::RDI},
            {"r8", Register::R8}, {"r9", Register::R9}, {"r10", Register::R10}, {"r11", Register::R11},
            {"r12", Register::R12}, {"r13", Register::R13}, {"r14", Register::R14}, {"r15", Register::R15},

            {"cr8", Register::CR8}, {"cr9", Register::CR9}, {"cr10", Register::CR10}, {"cr11", Register::CR11},
            {"cr12", Register::CR12}, {"cr13", Register::CR13}, {"cr14", Register::CR14}, {"cr15", Register::CR15},

            {"rflags", Register::RFLAGS},

            // FPU registers
            {"st0", Register::ST0}, {"st1", Register::ST1}, {"st2", Register::ST2}, {"st3", Register::ST3},
            {"st4", Register::ST4}, {"st5", Register::ST5}, {"st6", Register::ST6}, {"st7", Register::ST7},

            // MMX registers
            {"mm0", Register::MM0}, {"mm1", Register::MM1}, {"mm2", Register::MM2}, {"mm3", Register::MM3},
            {"mm4", Register::MM4}, {"mm5", Register::MM5}, {"mm6", Register::MM6}, {"mm7", Register::MM7},

            // XMM registers
            {"xmm0", Register::XMM0}, {"xmm1", Register::XMM1}, {"xmm2", Register::XMM2}, {"xmm3", Register::XMM3},
            {"xmm4", Register::XMM4}, {"xmm5", Register::XMM5}, {"xmm6", Register::XMM6}, {"xmm7", Register::XMM7},
            {"xmm8", Register::XMM8}, {"xmm9", Register::XMM9}, {"xmm10", Register::XMM10}, {"xmm11", Register::XMM11},
            {"xmm12", Register::XMM12}, {"xmm13", Register::XMM13}, {"xmm14", Register::XMM14}, {"xmm15", Register::XMM15},

            // YMM registers
            {"ymm0", Register::YMM0}, {"ymm1", Register::YMM1}, {"ymm2", Register::YMM2}, {"ymm3", Register::YMM3},
            {"ymm4", Register::YMM4}, {"ymm5", Register::YMM5}, {"ymm6", Register::YMM6}, {"ymm7", Register::YMM7},
            {"ymm8", Register::YMM8}, {"ymm9", Register::YMM9}, {"ymm10", Register::YMM10}, {"ymm11", Register::YMM11},
            {"ymm12", Register::YMM12}, {"ymm13", Register::YMM13}, {"ymm14", Register::YMM14}, {"ymm15", Register::YMM15},

            // ZMM registers
            {"zmm0", Register::ZMM0}, {"zmm1", Register::ZMM1}, {"zmm2", Register::ZMM2}, {"zmm3", Register::ZMM3},
            {"zmm4", Register::ZMM4}, {"zmm5", Register::ZMM5}, {"zmm6", Register::ZMM6}, {"zmm7", Register::ZMM7},
            {"zmm8", Register::ZMM8}, {"zmm9", Register::ZMM9}, {"zmm10", Register::ZMM10}, {"zmm11", Register::ZMM11},
            {"zmm12", Register::ZMM12}, {"zmm13", Register::ZMM13}, {"zmm14", Register::ZMM14}, {"zmm15", Register::ZMM15},
            {"zmm16", Register::ZMM16}, {"zmm17", Register::ZMM17}, {"zmm18", Register::ZMM18}, {"zmm19", Register::ZMM19},
            {"zmm20", Register::ZMM20}, {"zmm21", Register::ZMM21}, {"zmm22", Register::ZMM22}, {"zmm23", Register::ZMM23},
            {"zmm24", Register::ZMM24}, {"zmm25", Register::ZMM25}, {"zmm26", Register::ZMM26}, {"zmm27", Register::ZMM27},
            {"zmm28", Register::ZMM28}, {"zmm29", Register::ZMM29}, {"zmm30", Register::ZMM30}, {"zmm31", Register::ZMM31},

            // OpMask
            {"k0", Register::K0}, {"k1", Register::K1}, {"k2", Register::K2}, {"k3", Register::K3},
            {"k4", Register::K4}, {"k5", Register::K5}, {"k6", Register::K6}, {"k7", Register::K7},

            // SSE control and status
            {"mxcsr", Register::MXCSR},

            // Bound registers
            {"bnd0", Register::BND0}, {"bnd1", Register::BND1}, {"bnd2", Register::BND2}, {"bnd3", Register::BND3},
            {"bndcfgu", Register::BNDCFGU}, {"bndcfgs", Register::BNDCFGS},

            // other
            {"xcr0", Register::XCR0}, {"pkru", Register::PKRU},
        };
    }
}
