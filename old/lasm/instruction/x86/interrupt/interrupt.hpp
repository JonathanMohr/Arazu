#pragma once

#include "../instruction.hpp"

namespace Assembler
{
    namespace x86
    {
        class Simple_Interrupt_Instruction : public D_Instruction
        {
        public:
            Simple_Interrupt_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f);
            ~Simple_Interrupt_Instruction() override = default;
        };

        class Normal_Interrupt_Instruction : public D_Instruction
        {
        public:
            Normal_Interrupt_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f);
            ~Normal_Interrupt_Instruction() override = default;
        };
    }
}
