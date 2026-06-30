#include "objectwriter.hpp"

#include "binary/binary.hpp"
#include "elf/elf.hpp"

std::unique_ptr<Assembler::ObjectWriter> Assembler::ObjectWriter::Get(const Context& _context, Format format)
{
    switch (format)
    {
        case Format::Binary: return std::make_unique<BinaryWriter>(_context);
        case Format::ELF: return std::make_unique<ELFWriter>(_context);

        default: return nullptr;
    }
}

#include <array>
#include <limits>

void Assembler::ObjectWriter::WriteAll(const void* data, uint64_t size)
{
    const uint64_t maxChunk = static_cast<uint64_t>(std::numeric_limits<std::streamsize>::max());

    uint64_t written = 0;
    while (written < size)
    {
        uint64_t remaining = size - written;
        uint64_t chunk = std::min<uint64_t>(maxChunk, remaining);

        output->write(
            reinterpret_cast<const char*>(reinterpret_cast<const uint8_t*>(data) + written),
            static_cast<std::streamsize>(chunk)
        );

        if (output->fail())
            throw Exception::IOError("Could not write to output", 0, 0);

        written += chunk;
    }

    position += size;
}

void Assembler::ObjectWriter::WritePadding(uint64_t alignment)
{
    static constexpr uint64_t PAD_BUF_SIZE = 4096;
    static const std::array<uint8_t, PAD_BUF_SIZE> zeros{};

    uint64_t padding = (alignment - (position % alignment)) % alignment;
    while (padding > 0)
    {
        uint64_t chunk = std::min<uint64_t>(padding, PAD_BUF_SIZE);
        WriteAll(zeros.data(), chunk);
        padding -= chunk;
    }
}
