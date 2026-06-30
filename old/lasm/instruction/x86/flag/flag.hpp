#pragma once

#include "../instruction.hpp"

namespace Assembler
{
    namespace x86
    {
        class Simple_Flag_Instruction : public D_Instruction
        {
        public:
            Simple_Flag_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f);
            ~Simple_Flag_Instruction() override = default;
        };
    }
}
