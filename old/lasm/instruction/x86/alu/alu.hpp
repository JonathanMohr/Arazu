#pragma once

#include "../instruction.hpp"

namespace Assembler
{
    namespace x86
    {
        class Two_Argument_ALU_Instruction : public D_Instruction
        {
        public:
            Two_Argument_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& mainOperand, const AsmProg::Operand& otherOperand, uint64_t l, uint64_t c, StringPool::String f);
            ~Two_Argument_ALU_Instruction() override = default;

        protected:
            bool optimizeS() override;

            AsmProg::Operand mainOp;
            bool canOptimize = false;
            bool accumulatorReg = false;
        };

        class ADX_Alu_Instruction : public D_Instruction
        {
        public:
            ADX_Alu_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Register& regOperand, const AsmProg::Operand& otherOperand, uint64_t l, uint64_t c, StringPool::String f);
            ~ADX_Alu_Instruction() override = default;
        };

        class Mul_Div_ALU_Instruction : public D_Instruction
        {
        public:
            Mul_Div_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Register& reg, const AsmProg::Operand& operand, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f);
            Mul_Div_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Register& reg, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f);
            Mul_Div_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f);
            ~Mul_Div_ALU_Instruction() override = default;

        protected:
            bool optimizeS() override;

            bool canOptimize = false;
        };

        class Shift_Rotate_ALU_Instruction : public D_Instruction
        {
        public:
            Shift_Rotate_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, const AsmProg::Immediate& imm, uint64_t l, uint64_t c, StringPool::String f);
            // Use CL as count
            Shift_Rotate_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f);
            ~Shift_Rotate_ALU_Instruction() override = default;

        protected:
            bool optimizeS() override;

            bool canOptimize = false;
            bool is8Bit = false;
        };

        class One_Argument_ALU_Instruction : public D_Instruction
        {
        public:
            One_Argument_ALU_Instruction(x86::Instruction instr, BitMode bitmode, const AsmProg::Operand& operand, uint64_t l, uint64_t c, StringPool::String f);
            ~One_Argument_ALU_Instruction() override = default;
        };
    }
}
