#pragma once

#include "../instruction.hpp"

namespace Assembler
{
    namespace x86
    {
        class Simple_Control_Instruction : public D_Instruction
        {
        public:
            Simple_Control_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f);
            ~Simple_Control_Instruction() override = default;
        };

        class JMP_Instruction : public D_Instruction
        {
        public:
            JMP_Instruction(x86::Instruction instr, bool near, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f);
            JMP_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f);
            ~JMP_Instruction() override = default;

            bool optimizeS() override;

        private:
            x86::Instruction instruction;
            bool canOptimize = false;
        };

        class RET_Instruction : public D_Instruction
        {
        public:
            RET_Instruction(x86::Instruction instr, BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f);
            RET_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f);
            ~RET_Instruction() override = default;
        };

        class CALL_Instruction : public D_Instruction
        {
        public:
            CALL_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f);
            ~CALL_Instruction() override = default;
        };
    }
}
