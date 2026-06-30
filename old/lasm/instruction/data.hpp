#pragma once

#include "instruction.hpp"

namespace Assembler
{
    namespace Instructions
    {
        class Data : public Instruction
        {
        public:
            Data(uint64_t size, const std::vector<AsmProg::Immediate>& v, uint64_t l, uint64_t c, StringPool::String f);
            ~Data() = default;

            void encode(std::vector<uint8_t>& buffer, std::vector<Object::Relocation>* relocations, Evaluator::Context& context) override;
            uint64_t size() override;
            void evaluate(Evaluator::Context& context) override;
            bool optimize() override;

        private:
            uint64_t valueSize;
            std::vector<AsmProg::Immediate> values;

            uint64_t bufferSize;
            std::vector<uint8_t> internalBuffer;

            std::vector<Object::Relocation> relocBuffer;
        };

        class Reserved : public Instruction
        {
        public:
            Reserved(uint64_t size, const AsmProg::Immediate& v, StringPool::String empty, uint64_t l, uint64_t c, StringPool::String f);
            ~Reserved() = default;

            void encode(std::vector<uint8_t>& buffer, std::vector<Object::Relocation>* relocations, Evaluator::Context& context) override;
            uint64_t size() override;
            void evaluate(Evaluator::Context& context) override;
            bool optimize() override;

        private:
            uint64_t valueSize;
            AsmProg::Immediate value;

            uint64_t bufferSize;
            std::vector<uint8_t> internalBuffer;
        };
    }
}
