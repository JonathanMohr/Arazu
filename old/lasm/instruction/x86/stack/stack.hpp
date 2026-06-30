#pragma once

#include "../instruction.hpp"

namespace Assembler
{
    namespace x86
    {
        class Simple_Stack_Instruction : public D_Instruction
        {
        public:
            Simple_Stack_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f);
            ~Simple_Stack_Instruction() override = default;
        };

        class Normal_Stack_Instruction : public D_Instruction
        {
        public:
            Normal_Stack_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f);
            ~Normal_Stack_Instruction() override = default;

        protected:
            bool optimizeS() override;

            bool canOptimize = false;
        };
    }
}
