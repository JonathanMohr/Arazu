#pragma once

#include "../instruction.hpp"

namespace Assembler
{
    namespace x86
    {
        class Mov_Instruction : public D_Instruction
        {
        public:
            Mov_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& destinationOperand, const AsmProg::Operand& sourceOperand, uint64_t l, uint64_t c, StringPool::String f);
            ~Mov_Instruction() override = default;

        protected:
            bool optimizeS() override;

            bool canOptimize = false;
            bool needsREX = false;
            bool is8Bit = false;
            AsmProg::Register optimizeReg;
        };
    }
}
