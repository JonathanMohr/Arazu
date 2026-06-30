#include "data.hpp"

#include <Exception.hpp>
#include <limits>

Assembler::x86::Mov_Instruction::Mov_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& destinationOperand, const AsmProg::Operand& sourceOperand, uint64_t l, uint64_t c, StringPool::String f)
    : D_Instruction(bitmode, l, c, f)
{
    if (instr != x86::Instruction::MOV)
        throw Exception::InternalError("Invalid MOV instruction", line, column, file.c_str());

    bool usingSpecialReg = false;
    bool usingSegment = false;

    if (destinationOperand.isRegister())
    {
        if (!isRegGPR(destinationOperand.getRegister()))
            usingSpecialReg = true;
        if (isRegSegment(destinationOperand.getRegister()))
            usingSegment = true;
    }

    if (sourceOperand.isRegister())
    {
        if (!isRegGPR(sourceOperand.getRegister()))
            usingSpecialReg = true;
        if (isRegSegment(sourceOperand.getRegister()))
            usingSegment = true;
    }

    if (usingSpecialReg && !usingSegment)
    {
        if (!destinationOperand.isRegister())
            throw Exception::SyntaxError("Can only set special register to GPRs", line, column, file.c_str());
        if (!sourceOperand.isRegister())
            throw Exception::SyntaxError("Can only set GPRs to special registers", line, column, file.c_str());

        AsmProg::Register destReg = destinationOperand.getRegister();
        AsmProg::Register srcReg = sourceOperand.getRegister();

        bool usingSpecialRegDest = false;
        bool usingSpecialRegSrc = false;

        switch (destReg)
        {
            case Register::CR0:
            case Register::CR2:
            case Register::CR3:
            case Register::CR4:
            case Register::CR5:
            case Register::CR6:
            case Register::CR7:
                opcodeEscape = OpcodeEscape::TWO_BYTE;
                opcode = 0x22;

                usingSpecialRegDest = true;
                break;

            case Register::CR8:
            case Register::CR9:
            case Register::CR10:
            case Register::CR11:
            case Register::CR12:
            case Register::CR13:
            case Register::CR14:
            case Register::CR15:
                opcodeEscape = OpcodeEscape::TWO_BYTE;
                opcode = 0x22;

                rex.use = true;
                rex.r = true;

                usingSpecialRegDest = true;
                break;
            
            case Register::DR0:
            case Register::DR1:
            case Register::DR2:
            case Register::DR3:
            case Register::DR6:
            case Register::DR7:
                opcodeEscape = OpcodeEscape::TWO_BYTE;
                opcode = 0x23;

                usingSpecialRegDest = true;
                break;

            case Register::DR8:
            case Register::DR9:
            case Register::DR10:
            case Register::DR11:
            case Register::DR12:
            case Register::DR13:
            case Register::DR14:
            case Register::DR15:
                opcodeEscape = OpcodeEscape::TWO_BYTE;
                opcode = 0x23;

                rex.use = true;
                rex.r = true;

                usingSpecialRegDest = true;
                break;

            default:
                break;
        }

        switch (srcReg)
        {
            case Register::CR0:
            case Register::CR2:
            case Register::CR3:
            case Register::CR4:
            case Register::CR5:
            case Register::CR6:
            case Register::CR7:
                opcodeEscape = OpcodeEscape::TWO_BYTE;
                opcode = 0x20;

                usingSpecialRegSrc = true;
                break;

            case Register::CR8:
            case Register::CR9:
            case Register::CR10:
            case Register::CR11:
            case Register::CR12:
            case Register::CR13:
            case Register::CR14:
            case Register::CR15:
                opcodeEscape = OpcodeEscape::TWO_BYTE;
                opcode = 0x20;

                rex.use = true;
                rex.r = true;

                usingSpecialRegSrc = true;
                break;
            
            case Register::DR0:
            case Register::DR1:
            case Register::DR2:
            case Register::DR3:
            case Register::DR6:
            case Register::DR7:
                opcodeEscape = OpcodeEscape::TWO_BYTE;
                opcode = 0x21;

                usingSpecialRegSrc = true;
                break;

            case Register::DR8:
            case Register::DR9:
            case Register::DR10:
            case Register::DR11:
            case Register::DR12:
            case Register::DR13:
            case Register::DR14:
            case Register::DR15:
                opcodeEscape = OpcodeEscape::TWO_BYTE;
                opcode = 0x21;

                rex.use = true;
                rex.r = true;

                usingSpecialRegSrc = true;
                break;

            default:
                break;
        }

        if (usingSpecialRegDest && usingSpecialRegSrc)
            throw Exception::SemanticError("Cannot set special register to special register directly", line, column, file.c_str());

        uint64_t destRegSize;
        uint64_t srcRegSize;

        if (usingSpecialRegDest)
        {
            destRegSize = parseRegister(destReg, false, false);
            srcRegSize = parseRegister(srcReg, true, true);
        }
        else
        {
            destRegSize = parseRegister(destReg, true, true);
            srcRegSize = parseRegister(srcReg, false, false);
        }

        if (destRegSize != srcRegSize)
            throw Exception::SemanticError("Cannot use instruction with operands of different sizes", line, column, file.c_str());
    }
    else if (usingSegment)
    {
        bool useRMFirst = false;
        if (destinationOperand.isRegister())
        {
            if (sourceOperand.isRegister())
            {
                if (isRegSegment(destinationOperand.getRegister()) && isRegSegment(sourceOperand.getRegister()))
                    throw Exception::SyntaxError("Cannot set segment register to segment register directly", line, column, file.c_str());

                if (isRegSegment(sourceOperand.getRegister()))
                    useRMFirst = true;
                else
                    useRMFirst = false;
            }
            else if (sourceOperand.isMemory())
                useRMFirst = false;
            else
                throw Exception::SyntaxError("Cannot use immediates with segment registers", line, column, file.c_str());
        }
        else if (destinationOperand.isMemory())
        {
            if (sourceOperand.isRegister())
                useRMFirst = true;
            else if (sourceOperand.isMemory())
                throw Exception::SyntaxError("Cannot use two memory operands", line, column, file.c_str());
            else
                throw Exception::SyntaxError("Cannot use immediates with segment registers", line, column, file.c_str());
        }
        else
            throw Exception::SyntaxError("Cannot use immediate as destination operand", line, column, file.c_str());

        uint64_t destSize;
        uint64_t srcSize;

        if (destinationOperand.isRegister())
            destSize = parseRegister(destinationOperand.getRegister(), useRMFirst, useRMFirst);
        else
            destSize = parseMemory(destinationOperand.getMemory());

        if (sourceOperand.isRegister())
            srcSize = parseRegister(sourceOperand.getRegister(), !useRMFirst, !useRMFirst);
        else
            srcSize = parseMemory(sourceOperand.getMemory(), destSize);

        if (destSize == AsmProg::Memory::POINTER_SIZE_NONE)
            destSize = srcSize;

        const bool oneMem = (destinationOperand.isMemory() || sourceOperand.isMemory());

        if (oneMem && destSize != srcSize)
            throw Exception::SyntaxError("Can only use memory operand of size 16 with segment registers", line, column, file.c_str());

        opcode = (useRMFirst ? 0x8C : 0x8E);

        switch (destSize)
        {
            case 8:
                throw Exception::SemanticError("Cannot use 8-bit operand with segment registers", line, column, file.c_str());

            case 16:
                if (!oneMem && useRMFirst && bits != BitMode::Bits16) use16BitPrefix = true;
                break;

            case 32: case 64:
                break;

            default:
                throw Exception::InternalError("Invalid operand size", 0, 0);
        }
    }
    else
    {
        bool useRMFirst = false;
        bool otherIsImmediate = false;
        
        if (destinationOperand.isRegister())
        {
            if (sourceOperand.isRegister())
                useRMFirst = true;
            else if (sourceOperand.isMemory())
                useRMFirst = false;
            else
                otherIsImmediate = true;
        }
        else if (destinationOperand.isMemory())
        {
            if (sourceOperand.isRegister())
                useRMFirst = true;
            else if (sourceOperand.isMemory())
                throw Exception::SyntaxError("Cannot use two memory operands", line, column, file.c_str());
            else
                otherIsImmediate = true;
        }
        else
            throw Exception::SyntaxError("Cannot use immediate as destination operand", line, column, file.c_str());

        if (otherIsImmediate)
        {
            uint64_t destSize;
            if (destinationOperand.isRegister())
            {
                checkReg(destinationOperand.getRegister());
                destSize = getRegSize(destinationOperand.getRegister(), bits, line, column, file.c_str());
                checkSize(destSize);
            }
            else
                destSize = parseMemory(destinationOperand.getMemory(), AsmProg::Memory::POINTER_SIZE_NONE);

            immediate.use = true;
            immediate.immediate = sourceOperand.getImmediate();

            switch (destSize)
            {
                case 8:
                    immediate.sizeInBits = 8;
                    is8Bit = true;
                    break;

                case 16:
                    if (bits != BitMode::Bits16) use16BitPrefix = true;
                    immediate.sizeInBits = 16;
                    break;

                case 32:
                    if (bits == BitMode::Bits16) use16BitPrefix = true;
                    immediate.sizeInBits = 32;
                    break;

                case 64:
                    if (destinationOperand.isMemory())
                        immediate.sizeInBits = 32;
                    else
                        immediate.sizeInBits = 64;

                    rex.use = true;
                    rex.w = true;

                    if (destinationOperand.isRegister())
                    {
                        canOptimize = true;
                        optimizeReg = destinationOperand.getRegister();
                    }

                    break;

                default:
                    throw Exception::InternalError("Invalid operand size", line, column, file.c_str());
            }

            if (destinationOperand.isRegister())
            {
                auto [regIndex, regUseREX, regSetREX] = getRegInfo(destinationOperand.getRegister(), line, column, file.c_str());
                needsREX = regUseREX;

                if (regUseREX) rex.use = true;
                if (regSetREX) rex.b = true;

                if (destSize == 8)
                    opcode = 0xB0 + regIndex;
                else
                    opcode = 0xB8 + regIndex;
            }
            else
            {
                if (destSize == 8)
                    opcode = 0xC6;
                else
                    opcode = 0xC7;
            }
        }
        else
        {
            bool hasDirectMem = false;
            bool hasAccumulator = false;
            bool accumulatorFirst = false;

            uint64_t accumulatorSize;
            uint64_t directMemorySize;
            AsmProg::Immediate directMemoryImmediate;

            if (destinationOperand.isRegister())
            {
                AsmProg::Register reg = destinationOperand.getRegister();
                if (reg == Register::AL || reg == Register::AX || reg == Register::EAX || reg == Register::RAX)
                {
                    hasAccumulator = true;
                    accumulatorSize = getRegSize(reg, bits, line, column, file.c_str());
                    accumulatorFirst = true;
                    checkReg(reg);
                }
            }
            else
            {
                const AsmProg::Memory& mem = destinationOperand.getMemory();
                if (!mem.useReg1 && !mem.useReg2 && mem.useDisplacement)
                {
                    hasDirectMem = true;
                    directMemoryImmediate = mem.displacement;
                    directMemorySize = mem.pointerSize;
                }
            }

            if (sourceOperand.isRegister())
            {
                AsmProg::Register reg = sourceOperand.getRegister();
                if (reg == Register::AL || reg == Register::AX || reg == Register::EAX || reg == Register::RAX)
                {
                    hasAccumulator = true;
                    accumulatorSize = getRegSize(reg, bits, line, column, file.c_str());
                    checkReg(reg);
                }
            }
            else
            {
                const AsmProg::Memory& mem = sourceOperand.getMemory();
                if (!mem.useReg1 && !mem.useReg2 && mem.useDisplacement)
                {
                    hasDirectMem = true;
                    directMemoryImmediate = mem.displacement;
                    directMemorySize = mem.pointerSize;
                }
            }

            if (hasAccumulator && hasDirectMem && bits != BitMode::Bits64)
            {
                if (directMemorySize == AsmProg::Memory::POINTER_SIZE_NONE)
                    directMemorySize = accumulatorSize;

                if (directMemorySize != accumulatorSize)
                    throw Exception::SyntaxError("Cannot use operands of different sizes for instruction", line, column, file.c_str());

                checkSize(directMemorySize);

                if (bits == BitMode::Bits16)      addressMode = AddressMode::Bits16;
                else if (bits == BitMode::Bits32) addressMode = AddressMode::Bits32;
                else                              addressMode = AddressMode::Bits64;

                switch (directMemorySize)
                {
                    case 8:
                        opcode = (accumulatorFirst ? 0xA0 : 0xA2);
                        break;

                    case 16:
                        if (bits != BitMode::Bits16) use16BitPrefix = true;
                        opcode = (accumulatorFirst ? 0xA1 : 0xA3);
                        break;

                    case 32:
                        if (bits == BitMode::Bits16) use16BitPrefix = true;
                        opcode = (accumulatorFirst ? 0xA1 : 0xA3);
                        break;

                    case 64:
                        opcode = (accumulatorFirst ? 0xA1 : 0xA3);

                        rex.use = true;
                        rex.w = true;
                        break;

                    default:
                        throw Exception::InternalError("Invalid operand size", line, column, file.c_str());
                }

                displacement.use = true;
                displacement.immediate = directMemoryImmediate;
                displacement.can_optimize = false;
            }
            else
            {
                uint64_t destSize;
                uint64_t srcSize;

                if (destinationOperand.isRegister())
                    destSize = parseRegister(destinationOperand.getRegister(), true, useRMFirst);
                else
                    destSize = parseMemory(destinationOperand.getMemory());

                if (sourceOperand.isRegister())
                    srcSize = parseRegister(sourceOperand.getRegister(), true, !useRMFirst);
                else
                    srcSize = parseMemory(sourceOperand.getMemory(), destSize);

                if (destSize == AsmProg::Memory::POINTER_SIZE_NONE)
                    destSize = srcSize;

                if (destSize != srcSize)
                    throw Exception::SyntaxError("Cannot use operands of different sizes for instruction", line, column, file.c_str());

                switch (destSize)
                {
                    case 8:
                        opcode = (useRMFirst ? 0x88 : 0x8A);
                        break;

                    case 16:
                        if (bits != BitMode::Bits16) use16BitPrefix = true;
                        opcode = (useRMFirst ? 0x89 : 0x8B);
                        break;

                    case 32:
                        if (bits == BitMode::Bits16) use16BitPrefix = true;
                        opcode = (useRMFirst ? 0x89 : 0x8B);
                        break;

                    case 64:
                        opcode = (useRMFirst ? 0x89 : 0x8B);

                        rex.use = true;
                        rex.w = true;
                        break;

                    default:
                        throw Exception::InternalError("Invalid operand size", line, column, file.c_str());
                }
            }

            if (destinationOperand.isRegister()) checkRegFor8BitREX(destinationOperand.getRegister());
            if (sourceOperand.isRegister()) checkRegFor8BitREX(sourceOperand.getRegister());
        }
    }
}

bool Assembler::x86::Mov_Instruction::optimizeS()
{
    if (canOptimize && !immediate.needsRelocation)
    {
        int64_t value = static_cast<int64_t>(immediate.value);

        if (value <= static_cast<int64_t>(static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())))
        {
            if (value >= 0)
            {
                if (!needsREX) rex.use = false;
                rex.w = false;

                immediate.sizeInBits = 32;
                canOptimize = false;

                return true;
            }
            else if (value >= static_cast<int64_t>(static_cast<uint64_t>(std::numeric_limits<int32_t>().min())))
            {
                immediate.sizeInBits = 32;

                if (is8Bit) opcode = 0xC6;
                else        opcode = 0xC7;

                modrm.use = true;
                modrm.mod = Mod::REGISTER;
                modrm.reg = 0;
                auto [regIndex, ignore1, ignore2] = getRegInfo(optimizeReg, line, column, file.c_str());
                modrm.rm = regIndex;

                canOptimize = false;
                return true;
            }
        }
    }

    return false;
}
