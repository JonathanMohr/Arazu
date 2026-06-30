#pragma once

#include "../context.hpp"
#include "../object/object.hpp"

#include <memory>

namespace Assembler
{
    class ObjectWriter
    {
    public:
        ObjectWriter(const Context& _context) : context(_context) {}
        virtual ~ObjectWriter() = default;

        virtual void Write(const Object& object, std::ostream* output) = 0;

        static std::unique_ptr<ObjectWriter> Get(const Context& _context, Format format);

    protected:
        const Context& context;

        uint64_t position;
        std::ostream* output;

        void WriteAll(const void* data, uint64_t size);
        void WritePadding(uint64_t alignment);

        inline const std::vector<Object::Section>& getSections(const Object& object) { return object.sections; }
        inline const std::vector<Object::Symbol>& getSymbols(const Object& object) { return object.symbols; }

        inline Architecture getArchitecture(const Object& object) { return object.architecture; }
        inline BitMode getBits(const Object& object) { return object.bits; }
    };
}
