#include "instruction.hpp"

#include <cstring>
#include <Exception.hpp>

Assembler::x86::D_Instruction::D_Instruction(BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f)
    : Instruction(l, c, f), bits(bitmode)
{

}

uint64_t Assembler::x86::D_Instruction::getDisplacementOffset()
{
    uint64_t s = 0;

    if (use16BitPrefix) s++;
    if (use16BitAddressPrefix) s++;

    if (use66OpcodeOverride) s++;
    if (useF3OpcodeOverride) s++;

    if (rex.use) s++;

    switch (opcodeEscape)
    {
        case OpcodeEscape::NONE: break;
        case OpcodeEscape::TWO_BYTE: s++; break;
        case OpcodeEscape::THREE_BYTE_38: //s += 2; break;
        case OpcodeEscape::THREE_BYTE_3A: s += 2; break;
    }
    s++; // Opcode

    if (modrm.use) s++;
    if (sib.use) s++;

    return s;
}

uint64_t Assembler::x86::D_Instruction::getImmediateOffset()
{
    uint64_t s = getDisplacementOffset();

    if (displacement.use)
    {
        if (displacement.is_short) s++;
        else
        {
            if (addressMode == AddressMode::Bits16) s += 2;
            else                                    s += 4;
        }
    }
    else if (modrm.use && modrm.mod == Mod::INDIRECT_DISP8)
    {
        s++;
    }
    else if (modrm.use && modrm.mod == Mod::INDIRECT_DISP32)
    {
        if (addressMode == AddressMode::Bits16) s += 2;
        else                                    s += 4;
    }

    return s;
}

void Assembler::x86::D_Instruction::encode(std::vector<uint8_t>& buffer, std::vector<Object::Relocation>* relocations, Evaluator::Context& context)
{
    if (use16BitAddressPrefix) buffer.push_back(0x67);
    if (use16BitPrefix) buffer.push_back(0x66);

    // TODO: not happy
    if (use66OpcodeOverride) buffer.push_back(0x66);
    if (useF3OpcodeOverride) buffer.push_back(0xF3);

    if (rex.use) buffer.push_back(getRex(rex.w, rex.r, rex.x, rex.b));

    switch (opcodeEscape)
    {
        case OpcodeEscape::NONE:
            break;

        case OpcodeEscape::TWO_BYTE:
            buffer.push_back(0x0F);
            break;

        case OpcodeEscape::THREE_BYTE_38:
            buffer.push_back(0x0F);
            buffer.push_back(0x38);
            break;

        case OpcodeEscape::THREE_BYTE_3A:
            buffer.push_back(0x0F);
            buffer.push_back(0x3A);
            break;

        default:
            throw Exception::InternalError("Invalid opcodeEscape", line, column, file.c_str());
    }

    buffer.push_back(opcode);

    if (modrm.use)
    {
        if (sib.use)
            buffer.push_back(getModRM(modrm.mod, modrm.reg, modRMSIB));
        else
            buffer.push_back(getModRM(modrm.mod, modrm.reg, modrm.rm));
    }

    if (sib.use)
    {
        if (modrm.use) buffer.push_back(getSIB(sib.scale, sib.index, modrm.rm));
        else           throw Exception::InternalError("Can't use SIB without ModR/M", line, column, file.c_str());
    }

    if (displacement.use)
    {
        Object::Relocation::Size relocationSize;
        // TODO: Do it better!
        if (displacement.is_short)
        {
            buffer.push_back(static_cast<uint8_t>(displacement.value));
            relocationSize = Object::Relocation::Size::Bit8;
        }
        else
        {
            uint32_t sizeInBytes;
            if (addressMode == AddressMode::Bits16)
            {
                relocationSize = Object::Relocation::Size::Bit16;
                sizeInBytes = 2;
            }
            else
            {
                relocationSize = Object::Relocation::Size::Bit32;
                sizeInBytes = 4;
            }

            uint64_t oldSize = buffer.size();
            buffer.resize(oldSize + sizeInBytes);

            std::memcpy(buffer.data() + oldSize, &displacement.value, sizeInBytes);
        }

        if (displacement.needsRelocation && relocations)
        {
            Object::Relocation relocation;
            relocation.type = Object::Relocation::Type::Absolute;
            relocation.size = relocationSize;
            relocation.offsetInSection = context.sectionOffset + getDisplacementOffset();
            relocation.addend = displacement.value;
            relocation.usedSection = displacement.relocationUsedSection;
            relocation.isExtern = displacement.relocationIsExtern;
            if (addressMode == AddressMode::Bits64 || displacement.is_short)
                relocation.isSigned = true;
            else
                relocation.isSigned = false;
            relocations->push_back(relocation);
        }
    }
    else if (modrm.use && modrm.mod == Mod::INDIRECT_DISP8)
    {
        buffer.push_back(static_cast<uint8_t>(0));
    }
    else if (modrm.use && modrm.mod == Mod::INDIRECT_DISP32)
    {
        buffer.push_back(static_cast<uint8_t>(0));
        buffer.push_back(static_cast<uint8_t>(0));
        if (addressMode != AddressMode::Bits16)
        {
            buffer.push_back(static_cast<uint8_t>(0));
            buffer.push_back(static_cast<uint8_t>(0));
        }
    }

    if (immediate.use)
    {
        uint32_t sizeInBytes = immediate.sizeInBits / 8;

        uint64_t oldSize = buffer.size();
        buffer.resize(oldSize + sizeInBytes);

        std::memcpy(buffer.data() + oldSize, &immediate.value, sizeInBytes);

        if (immediate.needsRelocation && relocations)
        {
            Object::Relocation relocation;

            if (immediate.ripRelative)
                relocation.type = Object::Relocation::Type::PC_Relative;
            else
                relocation.type = Object::Relocation::Type::Absolute;
            
            switch (immediate.sizeInBits)
            {
                case 8: relocation.size = Object::Relocation::Size::Bit8; break;
                case 16: relocation.size = Object::Relocation::Size::Bit16; break;
                case 24: relocation.size = Object::Relocation::Size::Bit24; break;
                case 32: relocation.size = Object::Relocation::Size::Bit32; break;
                case 64: relocation.size = Object::Relocation::Size::Bit64; break;
                default:
                    throw Exception::InternalError("Invalid size in bits " + std::to_string(immediate.sizeInBits), line, column, file.c_str());
            }

            uint64_t value = immediate.value;
            if (immediate.ripRelative)
                value -= immediate.sizeInBits / 8;

            relocation.offsetInSection = context.sectionOffset + getImmediateOffset();
            relocation.addend = static_cast<int64_t>(value);
            relocation.usedSection = immediate.relocationUsedSection;
            relocation.isExtern = immediate.relocationIsExtern;
            relocation.isSigned = immediate.is_signed;
            
            relocations->push_back(relocation);
        }
    }

    encodeS(buffer, relocations);
}

uint64_t Assembler::x86::D_Instruction::size()
{
    uint64_t s = 0;

    if (use16BitPrefix) s++;
    if (use16BitAddressPrefix) s++;

    if (use66OpcodeOverride) s++;
    if (useF3OpcodeOverride) s++;

    if (rex.use) s++;

    switch (opcodeEscape)
    {
        case OpcodeEscape::NONE: break;
        case OpcodeEscape::TWO_BYTE: s++; break;
        case OpcodeEscape::THREE_BYTE_38: //s += 2; break;
        case OpcodeEscape::THREE_BYTE_3A: s += 2; break;
    }
    s++; // Opcode

    if (modrm.use) s++;
    if (sib.use) s++;

    if (displacement.use)
    {
        if (displacement.is_short) s++;
        else
        {
            if (addressMode == AddressMode::Bits16) s += 2;
            else                                    s += 4;
        }
    }
    else if (modrm.use && modrm.mod == Mod::INDIRECT_DISP8)
    {
        s++;
    }
    else if (modrm.use && modrm.mod == Mod::INDIRECT_DISP32)
    {
        if (addressMode == AddressMode::Bits16) s += 2;
        else                                    s += 4;
    }

    if (immediate.use)
        s += immediate.sizeInBits / 8;

    s += sizeS();

    return s;
}

void Assembler::x86::D_Instruction::evaluate(Evaluator::Context& context)
{
    // TODO: RIP-Relative
    if (displacement.use)
    {
        Evaluator::Result evaluation = Evaluator::Evaluate(displacement.immediate, context);

        if (evaluation.useOffset)
        {
            displacement.needsRelocation = true;

            displacement.relocationUsedSection = evaluation.usedSection;
            displacement.relocationIsExtern = evaluation.isExtern;

            displacement.can_optimize = false;
            displacement.value = evaluation.offset;
        }
        else
        {
            uint64_t result = evaluation.result;

            // TODO: Check for overflow

            displacement.value = static_cast<int64_t>(result);
        }
    }

    if (immediate.use)
    {
        uint64_t ripExtra = getImmediateOffset() + immediate.sizeInBits / 8;

        Evaluator::Result evaluation = Evaluator::Evaluate(immediate.immediate, context, immediate.ripRelative, ripExtra);

        if (evaluation.useOffset)
        {
            immediate.needsRelocation = true;

            immediate.relocationUsedSection = evaluation.usedSection;
            immediate.relocationIsExtern = evaluation.isExtern;

            immediate.value = static_cast<uint64_t>(evaluation.offset);
        }
        else
        {
            uint64_t result = evaluation.result;

            // TODO: Check for overflow

            immediate.value = static_cast<uint64_t>(result);
        }
    }

    evaluateS(context);
}

bool Assembler::x86::D_Instruction::optimize()
{
    bool changed = false;

    if (displacement.use && displacement.can_optimize && !displacement.is_short)
    {
        if (displacement.value >= -128 && displacement.value <= 127)
        {
            displacement.is_short = true;

            modrm.mod = Mod::INDIRECT_DISP8;

            changed = true;
        }
    }

    if (optimizeS())
        changed = true;

    return changed;
}

void Assembler::x86::D_Instruction::checkSize(uint64_t size)
{
    if (bits == BitMode::Bits64) return;
    if (size == 64)
        throw Exception::SyntaxError("64-bit size only supported in 64-bit mode", line, column, file.c_str());
}

void Assembler::x86::D_Instruction::checkReg(const AsmProg::Register& reg)
{
    if (bits == BitMode::Bits64) return;
    switch (reg)
    {
        case Register::SPL: case Register::BPL: case Register::SIL: case Register::DIL:
        case Register::R8B: case Register::R9B: case Register::R10B: case Register::R11B:
        case Register::R12B: case Register::R13B: case Register::R14B: case Register::R15B:
        case Register::R8W: case Register::R9W: case Register::R10W: case Register::R11W:
        case Register::R12W: case Register::R13W: case Register::R14W: case Register::R15W:
        case Register::R8D: case Register::R9D: case Register::R10D: case Register::R11D:
        case Register::R12D: case Register::R13D: case Register::R14D: case Register::R15D:
        case Register::RAX: case Register::RCX: case Register::RDX: case Register::RBX:
        case Register::RSP: case Register::RBP: case Register::RSI: case Register::RDI:
        case Register::R8: case Register::R9: case Register::R10: case Register::R11:
        case Register::R12: case Register::R13: case Register::R14: case Register::R15:

        case Register::CR8: case Register::CR9: case CR10:
        case Register::CR11: case Register::CR12: case CR13:
        case Register::CR14: case Register::CR15:

        case Register::DR8: case Register::DR9: case Register::DR10:
        case Register::DR11: case Register::DR12: case Register::DR13:
        case Register::DR14: case Register::DR15:
            throw Exception::SyntaxError("Register only supported in 64-bit mode", line, column, file.c_str());

        default:
            break;
    }
}

void Assembler::x86::D_Instruction::checkRegFor8BitREX(const AsmProg::Register& reg)
{
    if (reg == Register::AH || reg == Register::CH || reg == Register::DH || reg == Register::BH)
    {
        if (rex.use)
            throw Exception::SemanticError("Cannot use high 8-bit registers while using REX", line, column, file.c_str());
    }
}
