#include "x86.hpp"

#include <Exception.hpp>

std::tuple<uint8_t, bool, bool> Assembler::x86::getRegInfo(uint64_t reg, uint64_t line, uint64_t column, const char* file)
{
    switch (reg)
    {
        case AL:     return std::make_tuple((uint8_t)0, false, false);
        case CL:     return std::make_tuple((uint8_t)1, false, false);
        case DL:     return std::make_tuple((uint8_t)2, false, false);
        case BL:     return std::make_tuple((uint8_t)3, false, false);
        case AH:     return std::make_tuple((uint8_t)4, false, false);
        case CH:     return std::make_tuple((uint8_t)5, false, false);
        case DH:     return std::make_tuple((uint8_t)6, false, false);
        case BH:     return std::make_tuple((uint8_t)7, false, false);
        case SPL:    return std::make_tuple((uint8_t)4, true,  false);
        case BPL:    return std::make_tuple((uint8_t)5, true,  false);
        case SIL:    return std::make_tuple((uint8_t)6, true,  false);
        case DIL:    return std::make_tuple((uint8_t)7, true,  false);
        case R8B:    return std::make_tuple((uint8_t)0, true,  true);
        case R9B:    return std::make_tuple((uint8_t)1, true,  true);
        case R10B:   return std::make_tuple((uint8_t)2, true,  true);
        case R11B:   return std::make_tuple((uint8_t)3, true,  true);
        case R12B:   return std::make_tuple((uint8_t)4, true,  true);
        case R13B:   return std::make_tuple((uint8_t)5, true,  true);
        case R14B:   return std::make_tuple((uint8_t)6, true,  true);
        case R15B:   return std::make_tuple((uint8_t)7, true,  true);

        case AX:     return std::make_tuple((uint8_t)0, false, false);
        case CX:     return std::make_tuple((uint8_t)1, false, false);
        case DX:     return std::make_tuple((uint8_t)2, false, false);
        case BX:     return std::make_tuple((uint8_t)3, false, false);
        case SP:     return std::make_tuple((uint8_t)4, false, false);
        case BP:     return std::make_tuple((uint8_t)5, false, false);
        case SI:     return std::make_tuple((uint8_t)6, false, false);
        case DI:     return std::make_tuple((uint8_t)7, false, false);
        case R8W:    return std::make_tuple((uint8_t)0, true,  true);
        case R9W:    return std::make_tuple((uint8_t)1, true,  true);
        case R10W:   return std::make_tuple((uint8_t)2, true,  true);
        case R11W:   return std::make_tuple((uint8_t)3, true,  true);
        case R12W:   return std::make_tuple((uint8_t)4, true,  true);
        case R13W:   return std::make_tuple((uint8_t)5, true,  true);
        case R14W:   return std::make_tuple((uint8_t)6, true,  true);
        case R15W:   return std::make_tuple((uint8_t)7, true,  true);

        case ES:     return std::make_tuple((uint8_t)0, false,  false);
        case CS:     return std::make_tuple((uint8_t)1, false,  false);
        case SS:     return std::make_tuple((uint8_t)2, false,  false);
        case DS:     return std::make_tuple((uint8_t)3, false,  false);
        case FS:     return std::make_tuple((uint8_t)4, false,  false);
        case GS:     return std::make_tuple((uint8_t)5, false,  false);

        case EAX:    return std::make_tuple((uint8_t)0, false, false);
        case ECX:    return std::make_tuple((uint8_t)1, false, false);
        case EDX:    return std::make_tuple((uint8_t)2, false, false);
        case EBX:    return std::make_tuple((uint8_t)3, false, false);
        case ESP:    return std::make_tuple((uint8_t)4, false, false);
        case EBP:    return std::make_tuple((uint8_t)5, false, false);
        case ESI:    return std::make_tuple((uint8_t)6, false, false);
        case EDI:    return std::make_tuple((uint8_t)7, false, false);
        case R8D:    return std::make_tuple((uint8_t)0, true,  true);
        case R9D:    return std::make_tuple((uint8_t)1, true,  true);
        case R10D:   return std::make_tuple((uint8_t)2, true,  true);
        case R11D:   return std::make_tuple((uint8_t)3, true,  true);
        case R12D:   return std::make_tuple((uint8_t)4, true,  true);
        case R13D:   return std::make_tuple((uint8_t)5, true,  true);
        case R14D:   return std::make_tuple((uint8_t)6, true,  true);
        case R15D:   return std::make_tuple((uint8_t)7, true,  true);

        case RAX:    return std::make_tuple((uint8_t)0, false, false);
        case RCX:    return std::make_tuple((uint8_t)1, false, false);
        case RDX:    return std::make_tuple((uint8_t)2, false, false);
        case RBX:    return std::make_tuple((uint8_t)3, false, false);
        case RSP:    return std::make_tuple((uint8_t)4, false, false);
        case RBP:    return std::make_tuple((uint8_t)5, false, false);
        case RSI:    return std::make_tuple((uint8_t)6, false, false);
        case RDI:    return std::make_tuple((uint8_t)7, false, false);
        case R8:     return std::make_tuple((uint8_t)0, true,  true);
        case R9:     return std::make_tuple((uint8_t)1, true,  true);
        case R10:    return std::make_tuple((uint8_t)2, true,  true);
        case R11:    return std::make_tuple((uint8_t)3, true,  true);
        case R12:    return std::make_tuple((uint8_t)4, true,  true);
        case R13:    return std::make_tuple((uint8_t)5, true,  true);
        case R14:    return std::make_tuple((uint8_t)6, true,  true);
        case R15:    return std::make_tuple((uint8_t)7, true,  true);

        case CR0:    return std::make_tuple((uint8_t)0, false, false);
        case CR2:    return std::make_tuple((uint8_t)2, false, false);
        case CR3:    return std::make_tuple((uint8_t)3, false, false);
        case CR4:    return std::make_tuple((uint8_t)4, false, false);
        case CR5:    return std::make_tuple((uint8_t)5, false, false);
        case CR6:    return std::make_tuple((uint8_t)6, false, false);
        case CR7:    return std::make_tuple((uint8_t)7, false, false);
        case CR8:    return std::make_tuple((uint8_t)0, false, false);
        case CR9:    return std::make_tuple((uint8_t)1, false, false);
        case CR10:   return std::make_tuple((uint8_t)2, false, false);
        case CR11:   return std::make_tuple((uint8_t)3, false, false);
        case CR12:   return std::make_tuple((uint8_t)4, false, false);
        case CR13:   return std::make_tuple((uint8_t)5, false, false);
        case CR14:   return std::make_tuple((uint8_t)6, false, false);
        case CR15:   return std::make_tuple((uint8_t)7, false, false);

        case DR0:    return std::make_tuple((uint8_t)0, false, false);
        case DR1:    return std::make_tuple((uint8_t)1, false, false);
        case DR2:    return std::make_tuple((uint8_t)2, false, false);
        case DR3:    return std::make_tuple((uint8_t)3, false, false);
        case DR6:    return std::make_tuple((uint8_t)6, false, false);
        case DR7:    return std::make_tuple((uint8_t)7, false, false);
        case DR8:    return std::make_tuple((uint8_t)0, false, false);
        case DR9:    return std::make_tuple((uint8_t)1, false, false);
        case DR10:   return std::make_tuple((uint8_t)2, false, false);
        case DR11:   return std::make_tuple((uint8_t)3, false, false);
        case DR12:   return std::make_tuple((uint8_t)4, false, false);
        case DR13:   return std::make_tuple((uint8_t)5, false, false);
        case DR14:   return std::make_tuple((uint8_t)6, false, false);
        case DR15:   return std::make_tuple((uint8_t)7, false, false);

        default: throw Exception::InternalError("Invalid register", line, column, file);
    }
}

uint64_t Assembler::x86::getRegSize(uint64_t reg, BitMode mode, uint64_t line, uint64_t column, const char* file)
{
    switch (reg)
    {
        case Register::AL: case Register::CL:
        case Register::DL: case Register::BL:
        case Register::AH: case Register::CH:
        case Register::DH: case Register::BH:
        case Register::SPL: case Register::BPL:
        case Register::SIL: case Register::DIL:
        case Register::R8B: case Register::R9B:
        case Register::R10B: case Register::R11B:
        case Register::R12B: case Register::R13B:
        case Register::R14B: case Register::R15B:
            return 8;

        case Register::AX: case Register::CX:
        case Register::DX: case Register::BX:
        case Register::SP: case Register::BP:
        case Register::SI: case Register::DI:
        case Register::R8W: case Register::R9W:
        case Register::R10W: case Register::R11W:
        case Register::R12W: case Register::R13W:
        case Register::R14W: case Register::R15W:
            //return 16;

        case Register::ES: case Register::CS:
        case Register::SS: case Register::DS:
        case Register::FS: case Register::GS:
            return 16;

        case Register::EAX: case Register::EBX:
        case Register::ECX: case Register::EDX:
        case Register::ESP: case Register::EBP:
        case Register::ESI: case Register::EDI:
        case Register::R8D: case Register::R9D:
        case Register::R10D: case Register::R11D:
        case Register::R12D: case Register::R13D:
        case Register::R14D: case Register::R15D:
            return 32;

        case Register::RAX: case Register::RBX:
        case Register::RCX: case Register::RDX:
        case Register::RSP: case Register::RBP:
        case Register::RSI: case Register::RDI:
        case Register::R8: case Register::R9:
        case Register::R10: case Register::R11:
        case Register::R12: case Register::R13:
        case Register::R14: case Register::R15:
            return 64;


        case Register::CR0: case Register::CR2:
        case Register::CR3: case Register::CR4:
        case Register::CR5: case Register::CR6:
        case Register::CR7: case Register::CR8:
        case Register::CR9: case Register::CR10:
        case Register::CR11: case Register::CR12:
        case Register::CR13: case Register::CR14:
        case Register::CR15:
        case Register::DR0: case Register::DR1:
        case Register::DR2: case Register::DR3:
        case Register::DR6: case Register::DR7:
        case Register::DR8: case Register::DR9:
        case Register::DR10: case Register::DR11:
        case Register::DR12: case Register::DR13:
        case Register::DR14: case Register::DR15:
            if (mode == BitMode::Bits64) return 64;
            else return 32;

        default: throw Exception::InternalError("Invalid register", line, column, file);
    }
}

bool Assembler::x86::isRegGPR(uint64_t reg)
{
    switch (reg)
    {
        case Register::AL: case Register::CL: case Register::DL: case Register::BL:
        case Register::AH: case Register::CH: case Register::DH: case Register::BH:
        case Register::SPL: case Register::BPL: case Register::SIL: case Register::DIL:
        case Register::R8B: case Register::R9B: case Register::R10B: case Register::R11B:
        case Register::R12B: case Register::R13B: case Register::R14B: case Register::R15B:

        case Register::AX: case Register::CX: case Register::DX: case Register::BX:
        case Register::SP: case Register::BP: case Register::SI: case Register::DI:
        case Register::R8W: case Register::R9W: case Register::R10W: case Register::R11W:
        case Register::R12W: case Register::R13W: case Register::R14W: case Register::R15W:

        case Register::EAX: case Register::ECX: case Register::EDX: case Register::EBX:
        case Register::ESP: case Register::EBP: case Register::ESI: case Register::EDI:
        case Register::R8D: case Register::R9D: case Register::R10D: case Register::R11D:
        case Register::R12D: case Register::R13D: case Register::R14D: case Register::R15D:

        case Register::RAX: case Register::RCX: case Register::RDX: case Register::RBX:
        case Register::RSP: case Register::RBP: case Register::RSI: case Register::RDI:
        case Register::R8: case Register::R9: case Register::R10: case Register::R11:
        case Register::R12: case Register::R13: case Register::R14: case Register::R15:
            return true;

        default:
            return false;
    }
}

bool Assembler::x86::isRegSegment(uint64_t reg)
{
    switch (reg)
    {
        case Register::ES: case Register::CS: case Register::SS:
        case Register::DS: case Register::FS: case Register::GS:
            return true;

        default:
            return false;
    }
}
