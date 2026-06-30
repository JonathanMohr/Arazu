#pragma once

#include "../objectwriter.hpp"

namespace Assembler
{
    class BinaryWriter : public ObjectWriter
    {
    public:
        BinaryWriter(const Context& _context) : ObjectWriter(_context) {}

        void Write(const Object& object, std::ostream* output) override;
    };
}
