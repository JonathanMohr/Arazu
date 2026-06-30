#pragma once

#include <Architecture.hpp>
#include <StringPool.hpp>
#include <cstdint>
#include <optional>

namespace Assembler
{
    class Object
    {
    public:
        struct Relocation
        {
            enum class Type : uint8_t
            {
                Absolute,
                PC_Relative
            };

            enum class Size : uint8_t
            {
                Bit8,
                Bit16,
                Bit24,
                Bit32,
                Bit64
            };

            Type type;
            Size size;

            uint64_t offsetInSection;
            int64_t addend;

            StringPool::String usedSection;
            bool isExtern;

            bool isSigned;
        };

        struct Symbol
        {
            StringPool::String name;
            uint64_t value;

            // nullopt: absolute specifies whether undefined
            std::optional<StringPool::String> section;
            bool absolute;

            bool isGlobal;

            uint64_t line;
            uint64_t column;
            StringPool::String file;
        };

        struct Section
        {
            using Flags = uint8_t;
            static constexpr Flags FlagsAllocatable = Flags{1u << 0};
            static constexpr Flags FlagsExecutable  = Flags{1u << 1};
            static constexpr Flags FlagsWritable    = Flags{1u << 2};

            enum class Type : uint8_t
            {
                Initialized,
                Uninitialized
            };

            StringPool::String name;
            uint64_t align;
            
            Flags flags;
            Type type;

            uint64_t size;
            std::vector<uint8_t> buffer;

            std::vector<Relocation> relocations;
        };
        
    private:
        std::vector<Section> sections;
        std::vector<Symbol> symbols;

        Architecture architecture;
        BitMode bits;

        friend class Encoder;
        friend class ObjectWriter;
    };
}
