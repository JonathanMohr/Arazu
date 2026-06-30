#include "instruction.hpp"

#include <Exception.hpp>

uint64_t Assembler::x86::D_Instruction::parseMemory(const AsmProg::Memory& memory)
{
    modrm.use = true;
    modrm.mod = Mod::INDIRECT;

    uint64_t mem_reg_size;

    if (memory.useReg1 && memory.useReg2)
    {
        uint64_t base_reg_size = getRegSize(memory.reg1, bits, line, column, file.c_str());
        uint64_t index_reg_size = getRegSize(memory.reg2, bits, line, column, file.c_str());
        if (base_reg_size != index_reg_size)
            throw Exception::SyntaxError("Base register and index register have different sizes", line, column, file.c_str());
        mem_reg_size = base_reg_size;
    }
    else if (memory.useReg1 && !memory.useReg2)
        mem_reg_size = getRegSize(memory.reg1, bits, line, column, file.c_str());
    else if (!memory.useReg1 && memory.useReg2)
        mem_reg_size = getRegSize(memory.reg2, bits, line, column, file.c_str());
    else
    {
        if (bits == BitMode::Bits16)
            mem_reg_size = 16;
        else if (bits == BitMode::Bits32)
            mem_reg_size = 32;
        else
            mem_reg_size = 64;
    }

    switch (mem_reg_size)
    {
        case 16:
            addressMode = AddressMode::Bits16;
            break;

        case 32:
            addressMode = AddressMode::Bits32;
            break;

        case 64:
            addressMode = AddressMode::Bits64;
            break;

        default:
            throw Exception::SemanticError("Memory operand registers need to be 16, 32 or 64 bits in size", line, column, file.c_str());
    }

    bool hasBase = false;
    AsmProg::Register baseReg;

    bool hasIndex = false;
    AsmProg::Register indexReg;
    uint64_t scale;

    // TODO: ugly
    StringPool stringPool;
    std::vector<AsmProg::Symbol> emptySymbols;
    std::unordered_map<std::string, uint64_t> emptyLabelMap;
    std::unordered_map<std::string, uint64_t> emptyConstantMap;
    std::unordered_map<std::string, uint64_t> emptySectionStart;
    Evaluator::Context evaluatorContext(
        line, column, file,
        emptySymbols, emptyLabelMap, emptyConstantMap,
        0, 0,
        stringPool.empty(), emptySectionStart
    );

    // TODO: Think of better way
    if (memory.useReg1 && memory.useReg2)
    {
        Evaluator::Result reg1Evaluation = Evaluator::Evaluate(memory.scale1, evaluatorContext);
        Evaluator::Result reg2Evaluation = Evaluator::Evaluate(memory.scale2, evaluatorContext);

        if (reg1Evaluation.useOffset || reg2Evaluation.useOffset)
            throw Exception::SemanticError("Scale must be a constant", line, column, file.c_str());

        const uint64_t reg1Result = reg1Evaluation.result;
        const uint64_t reg2Result = reg2Evaluation.result;

        hasBase = true;
        hasIndex = true;

        if (reg1Result == 1 && reg2Result == 1)
        {
            if (memory.reg2 == Register::ESP || memory.reg2 == Register::RSP ||
                memory.reg2 == Register::BX || memory.reg2 == Register::BP)
            {
                baseReg = memory.reg2;

                indexReg = memory.reg1;
                scale = reg1Result;
            }
            else /* memory.reg1 == Register::ESP || memory.reg1 == Register::RSP ||
                    memory.reg1 == Register::BX || memory.reg1 == Register::BP */
            {
                baseReg = memory.reg1;

                indexReg = memory.reg2;
                scale = reg2Result;
            }
        }
        else if (reg1Result == 1 && reg2Result != 1)
        {
            baseReg = memory.reg1;

            indexReg = memory.reg2;
            scale = reg2Result;
        }
        else if (reg1Result != 1 && reg2Result == 1)
        {
            baseReg = memory.reg2;

            indexReg = memory.reg1;
            scale = reg1Result;
        }
        else
            throw Exception::SemanticError("Can't use 2 index registers", line, column, file.c_str());
    }
    else if ((memory.useReg1 && !memory.useReg2) || (!memory.useReg1 && memory.useReg2))
    {
        AsmProg::Register usedReg;
        const AsmProg::Immediate* usedScale;

        if (memory.useReg1)
        {
            usedReg = memory.reg1;
            usedScale = &memory.scale1;
        }
        else // memory.useReg2
        {
            usedReg = memory.reg2;
            usedScale = &memory.scale2;
        }

        Evaluator::Result evaluation = Evaluator::Evaluate(*usedScale, evaluatorContext);

        if (evaluation.useOffset)
            throw Exception::SemanticError("Scale must be a constant", line, column, file.c_str());

        uint64_t result = evaluation.result;

        if (addressMode == AddressMode::Bits16)
        {
            scale = result;

            if (usedReg == Register::BX || usedReg == Register::BP)
            {
                hasBase = true;
                baseReg = usedReg;
            }
            else
            {
                hasIndex = true;
                indexReg = usedReg;
            }
        }
        else
        {
            switch (result)
            {
                case 1:
                    hasBase = true;
                    baseReg = usedReg;
                    break;

                case 2:
                    hasBase = true;
                    baseReg = usedReg;

                    hasIndex = true;
                    indexReg = usedReg;
                    scale = 1;
                    break;

                case 3:
                    hasBase = true;
                    baseReg = usedReg;

                    hasIndex = true;
                    indexReg = usedReg;
                    scale = 2;
                    break;

                case 4:
                    hasIndex = true;
                    indexReg = usedReg;
                    scale = 4;
                    break;

                case 5:
                    hasBase = true;
                    baseReg = usedReg;

                    hasIndex = true;
                    indexReg = usedReg;
                    scale = 4;
                    break;

                case 8:
                    hasIndex = true;
                    indexReg = usedReg;
                    scale = 8;
                    break;

                case 9:
                    hasBase = true;
                    baseReg = usedReg;

                    hasIndex = true;
                    indexReg = usedReg;
                    scale = 8;
                    break;

                case 6: case 7: default:
                    throw Exception::SemanticError("Scale needs to be 1, 2, 4, or 8", line, column, file.c_str());
            }
        }
    }

    if (hasBase)
    {
        if (!isRegGPR(baseReg))
            throw Exception::SyntaxError("Only GPRs are allowed for memory operands", line, column, file.c_str());
    }
    if (hasIndex)
    {
        if (!isRegGPR(indexReg))
            throw Exception::SyntaxError("Only GPRs are allowed for memory operands", line, column, file.c_str());

        if (addressMode == AddressMode::Bits16)
        {
            if (scale != 1)
                throw Exception::SemanticError("Scale must be 1 when using 16-bit addressing", line, column, file.c_str());
        }

        switch (scale)
        {
            case 1: sib.scale = Scale::x1; break;
            case 2: sib.scale = Scale::x2; break;
            case 4: sib.scale = Scale::x4; break;
            case 8: sib.scale = Scale::x8; break;
            default:
                throw Exception::SemanticError("Scale needs to be 1, 2, 4, or 8", line, column, file.c_str());
        }
    }

    if (addressMode == AddressMode::Bits16)
    {
        if (bits == BitMode::Bits64)
            throw Exception::SemanticError("64-bit registers are only allowed in 64-bit mode", line, column, file.c_str());
        else if (bits == BitMode::Bits32)
            use16BitAddressPrefix = true;

        if (hasBase && (baseReg != Register::BX && baseReg != Register::BP))
            throw Exception::SemanticError("Only bx/bp allowed as 16-bit base", line, column, file.c_str());
        
        if (hasIndex && (indexReg != Register::SI && indexReg != Register::DI))
            throw Exception::SemanticError("Only si/di allowed as 16-bit index", line, column, file.c_str());

        bool noBase = false;
        uint8_t addressing_mode;
        if (hasBase && hasIndex)
        {
            // [bx+si], [bx+di], [bp+si], [bp+di]
            if (baseReg == Register::BX && indexReg == Register::SI) addressing_mode = 0;  // 000b
            else if (baseReg == Register::BX && indexReg == Register::DI) addressing_mode = 1;  // 001b
            else if (baseReg == Register::BP && indexReg == Register::SI) addressing_mode = 2;  // 010b
            else if (baseReg == Register::BP && indexReg == Register::DI) addressing_mode = 3;  // 011b
            else
                throw Exception::SemanticError("Invalid 16-bit addressing combination", line, column, file.c_str());
        }
        else if (hasBase && !hasIndex)
        {
            // [bx], [bp]
            if (baseReg == Register::BX) addressing_mode = 7;  // 111b
            else if (baseReg == Register::BP) addressing_mode = 6; // 110b
            else
                throw Exception::SemanticError("Invalid 16-bit base register", line, column, file.c_str());
        }
        else if (!hasBase && hasIndex)
        {
            // [si], [di]
            if (indexReg == Register::SI) addressing_mode = 4;  // 100b
            else if (indexReg == Register::DI) addressing_mode = 5;  // 101b
            else
                throw Exception::SemanticError("Invalid 16-bit index register", line, column, file.c_str());
        }
        else
        {
            noBase = true;
            addressing_mode = 6;  // Direct address
        }

        modrm.rm = addressing_mode;

        if (memory.useDisplacement)
        {
            displacement.use = true;
            displacement.immediate = memory.displacement;

            if (noBase)  // Direct address
            {
                modrm.mod = Mod::INDIRECT;
                displacement.can_optimize = false;
                displacement.direct_addressing = true;
            }
            else if (!hasBase && hasIndex)
            {
                modrm.mod = Mod::INDIRECT_DISP8;
            }
            else
                modrm.mod = Mod::INDIRECT_DISP32;
        }
        else if (addressing_mode == 6 && !noBase)
        {
            modrm.mod = Mod::INDIRECT_DISP8;
        }
    }
    else
    {
        if (addressMode == AddressMode::Bits32 && bits != BitMode::Bits32)
            use16BitAddressPrefix = true;

        if (addressMode == AddressMode::Bits64)
        {
            if (bits != BitMode::Bits64)
                throw Exception::SemanticError("64-bit registers are only allowed in 64-bit mode", line, column, file.c_str());
        }

        if (hasBase)
        {
            auto [baseIndex, baseUseREX, baseSetREX] = getRegInfo(baseReg, line, column, file.c_str());

            if (baseUseREX) rex.use = true;
            if (baseSetREX) rex.b = true;

            modrm.rm = baseIndex;

            sib.index = SIB_NoIndex;
            if (baseIndex == modRMSIB) sib.use = true;

            if (baseIndex == modRMDisp)
                modrm.mod = Mod::INDIRECT_DISP8;
        }
        else if (hasIndex)
        {
            modrm.mod = Mod::INDIRECT;
            modrm.rm = modRMDisp;
        }

        if (hasIndex)
        {
            auto [indexIndex, indexUseREX, indexSetREX] = getRegInfo(indexReg, line, column, file.c_str());

            if (indexIndex == SIB_NoIndex && !indexSetREX)
                throw Exception::SemanticError("Can't use ESP/RSP as index register", line, column, file.c_str());

            if (indexUseREX) rex.use = true;
            if (indexSetREX) rex.x = true;

            sib.use = true;
            sib.index = indexIndex;
        }

        if (memory.useDisplacement)
        {
            if (hasBase) modrm.mod = Mod::INDIRECT_DISP32;

            displacement.use = true;
            displacement.immediate = memory.displacement;

            if (!hasBase && !hasIndex)
            {
                displacement.direct_addressing = true;

                if (addressMode == AddressMode::Bits64)
                {
                    modrm.mod = Mod::INDIRECT;
                    modrm.rm = modRMDisp;

                    sib.use = true;
                    sib.scale = Scale::x1;
                    sib.index = SIB_NoIndex;

                    displacement.can_optimize = false;
                }
                else
                {
                    modrm.mod = Mod::INDIRECT;
                    modrm.rm = modRMDisp;

                    displacement.can_optimize = false;
                }
            }
        }
    }

    checkSize(memory.pointerSize);

    return memory.pointerSize;
}

uint64_t Assembler::x86::D_Instruction::parseMemory(const AsmProg::Memory& memory, uint64_t implicitSize)
{
    const uint64_t size = parseMemory(memory);
    if (size == AsmProg::Memory::POINTER_SIZE_NONE)
    {
        if (implicitSize == AsmProg::Memory::POINTER_SIZE_NONE)
            throw Exception::SyntaxError("Pointer size not specified for memory operand", line, column, file.c_str());
        else
            return implicitSize;
    }
    return size;
}

uint64_t Assembler::x86::D_Instruction::parseRegister(const AsmProg::Register& reg, bool forceGPR, bool isRM)
{
    if (forceGPR && !isRegGPR(reg))
        throw Exception::SyntaxError("Instruction only supports GPRs", line, column, file.c_str());

    checkReg(reg);

    modrm.use = true;

    auto [regIndex, regUseREX, regSetREX] = getRegInfo(reg, line, column, file.c_str());
    uint64_t regSize = getRegSize(reg, bits, line, column, file.c_str());

    if (regUseREX) rex.use = true;
    if (regSetREX)
    {
        if (isRM) rex.b = true;
        else      rex.r = true;
    }

    if (isRM) modrm.rm = regIndex;
    else      modrm.reg = regIndex;

    checkSize(regSize);

    return regSize;
}
