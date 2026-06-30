#pragma once

#include "../instruction.hpp"

namespace Assembler
{
    namespace x86
    {
        class ASCII_Instruction : public D_Instruction
        {
        public:
            ASCII_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f);
            ASCII_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f);
            ~ASCII_Instruction() override = default;
        };
    }
}
