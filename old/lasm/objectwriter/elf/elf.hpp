#pragma once

#include "../objectwriter.hpp"

namespace Assembler
{
    class ELFWriter : public ObjectWriter
    {
    public:
        ELFWriter(const Context& _context) : ObjectWriter(_context) {}

        void Write(const Object& object, std::ostream* _output) override;

    private:
        bool useELF32;

        void WriteNullSectionHeader();
    };
}
