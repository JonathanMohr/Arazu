#include "binary.hpp"

#include <cstring>
#include <algorithm>

void Assembler::BinaryWriter::Write(const Object& object, std::ostream* _output)
{
    position = 0;
    output = _output;
    std::vector<Object::Section> sections = getSections(object);

    std::sort(sections.begin(), sections.end(), [](const Object::Section& a, const Object::Section& b) {
        if (a.type != b.type)
            return a.type == Object::Section::Type::Initialized;
        return (a.flags & Object::Section::FlagsExecutable) != 0;
    });

    std::unordered_map<std::string, uint64_t> sectionOffsets;

    uint64_t off = 0;
    for (const Object::Section& section : sections)
    {
        if (section.align == 0)
            throw Exception::InternalError("Alignment is 0 for section '" + section.name.str() + "'", 0, 0);

        off = (off + section.align - 1) / section.align * section.align;

        switch (section.type)
        {
            case Object::Section::Type::Initialized: case Object::Section::Type::Uninitialized:
            {
                sectionOffsets[section.name.str()] = off;
                off += section.size;
                break;
            }

            default:
                throw Exception::InternalError("Invalid section type for section '" + section.name.str() + "'", 0, 0);
        }
    }

    for (Object::Section& section : sections)
    {
        for (const Object::Relocation& relocation : section.relocations)
        {
            if (relocation.isExtern)
                throw Exception::SemanticError("Cannot use external symbols with binary output", 0, 0); // TODO: line, column

            auto usedIt = sectionOffsets.find(relocation.usedSection.str());
            if (usedIt == sectionOffsets.end())
                throw Exception::InternalError("Could not find section '" + section.name.str() + "'", 0, 0);

            auto selfIt = sectionOffsets.find(section.name.str());
            if (selfIt == sectionOffsets.end())
                throw Exception::InternalError("Could not find section '" + section.name.str() + "'", 0, 0);

            const uint64_t value = usedIt->second + static_cast<uint64_t>(relocation.addend);
            const uint64_t& offset = relocation.offsetInSection;

            // TODO: Check for overflow
            switch (relocation.type)
            {
                case Object::Relocation::Type::Absolute:
                {
                    switch (relocation.size)
                    {
                        case Object::Relocation::Size::Bit8:
                        {
                            const uint8_t val = static_cast<uint8_t>(value);
                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        case Object::Relocation::Size::Bit16:
                        {
                            const uint16_t val = static_cast<uint16_t>(value);
                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        case Object::Relocation::Size::Bit24:
                        {
                            const uint32_t val32 = static_cast<uint32_t>(value);

                            // Little-endian
                            uint8_t val[3];
                            val[0] = static_cast<uint8_t>(val32 & 0xFF);
                            val[1] = static_cast<uint8_t>((val32 >> 8) & 0xFF);
                            val[2] = static_cast<uint8_t>((val32 >> 16) & 0xFF);

                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        case Object::Relocation::Size::Bit32:
                        {
                            const uint32_t val = static_cast<uint32_t>(value);
                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        case Object::Relocation::Size::Bit64:
                        {
                            const uint64_t val = static_cast<uint64_t>(value);
                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        default:
                            throw Exception::InternalError("Invalid relocation size", 0, 0);
                    }

                    break;
                }

                case Object::Relocation::Type::PC_Relative:
                {
                    const uint64_t pos = selfIt->second + offset;
                    const int64_t patchedValue = static_cast<int64_t>(value + (~pos + 1));

                    switch (relocation.size)
                    {
                        case Object::Relocation::Size::Bit8:
                        {
                            const uint8_t val = static_cast<uint8_t>(patchedValue);
                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        case Object::Relocation::Size::Bit16:
                        {
                            const uint16_t val = static_cast<uint16_t>(patchedValue);
                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        case Object::Relocation::Size::Bit24:
                        {
                            const uint32_t val32 = static_cast<uint32_t>(patchedValue);

                            // Little-endian
                            uint8_t val[3];
                            val[0] = static_cast<uint8_t>(val32 & 0xFF);
                            val[1] = static_cast<uint8_t>((val32 >> 8) & 0xFF);
                            val[2] = static_cast<uint8_t>((val32 >> 16) & 0xFF);

                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        case Object::Relocation::Size::Bit32:
                        {
                            const uint32_t val = static_cast<uint32_t>(patchedValue);
                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        case Object::Relocation::Size::Bit64:
                        {
                            const uint64_t val = static_cast<uint64_t>(patchedValue);
                            std::memcpy(section.buffer.data() + offset, &val, sizeof(val));
                            break;
                        }

                        default:
                            throw Exception::InternalError("Invalid relocation size", 0, 0);
                    }

                    break;
                }

                default:
                    throw Exception::InternalError("Invalid relocation type", 0, 0);
            }
        }
    }

    uint64_t offset = 0;
    for (const Object::Section& section : sections)
    {
        if (section.buffer.empty()) continue;

        switch (section.type)
        {
            case Object::Section::Type::Initialized:
            {
                WritePadding(section.align);
                WriteAll(section.buffer.data(), section.size);
                offset += section.size;
                break;
            }

            case Object::Section::Type::Uninitialized:
            {
                uint64_t padding = (section.align - (offset % section.align)) % section.align;
                offset += padding;
                offset += section.size;
                break;
            }

            default:
                throw Exception::InternalError("Invalid section type for section '" + section.name.str() + "'", 0, 0);
        }
    }
}
