#pragma once

#include "evaluate.hpp"
#include "../asmprog/asmprog.hpp"
#include "../object/object.hpp"

namespace Assembler
{
    class Instruction
    {
    public:
        Instruction(uint64_t l, uint64_t c, StringPool::String f);
        virtual ~Instruction() = default;

        virtual void encode(std::vector<uint8_t>& buffer, std::vector<Object::Relocation>* relocations, Evaluator::Context& context) = 0;
        virtual uint64_t size() = 0;
        virtual void evaluate(Evaluator::Context& context) = 0;
        virtual bool optimize() = 0;

        inline uint64_t getLine() { return line; }
        inline uint64_t getColumn() { return column; }
        inline StringPool::String getFile() { return file; }

    protected:
        uint64_t line;
        uint64_t column;
        StringPool::String file;
    };
}
