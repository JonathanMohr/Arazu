#include "data.hpp"

#include <Exception.hpp>

Assembler::Instructions::Data::Data(uint64_t size, const std::vector<AsmProg::Immediate>& v, uint64_t l, uint64_t c, StringPool::String f)
    : Instruction(l, c, f), valueSize(size), values(v), bufferSize(valueSize * values.size())
{
    if (values.empty())
        throw Exception::SyntaxError("Missing values after data definition", line, column, file.c_str());

    switch (valueSize)
    {
        case 1: case 2: case 4: case 8:
            break;

        default:
            throw Exception::SyntaxError("Initialized data can only be initializes with a length of 1, 2, 4 or 8 bytes", line, column, file.c_str());
    }

    for (const AsmProg::Immediate& value : values)
    {
        if (value.operands.empty())
            throw Exception::SyntaxError("Data definition is empty", line, column, file.c_str());
    }
}

void Assembler::Instructions::Data::encode(std::vector<uint8_t>& buffer, std::vector<Object::Relocation>* relocations, Evaluator::Context& context)
{
    (void)context;
    if (relocations) relocations->insert(relocations->end(), relocBuffer.begin(), relocBuffer.end());
    buffer.insert(buffer.end(), internalBuffer.begin(), internalBuffer.end());
}

uint64_t Assembler::Instructions::Data::size()
{
    return bufferSize;
}

void Assembler::Instructions::Data::evaluate(Evaluator::Context& context)
{
    context.line = line;
    context.column = column;
    context.file = file;

    internalBuffer.clear();

    uint64_t offset = 0;
    for (const AsmProg::Immediate& value : values)
    {
        Evaluator::Result evaluation = Evaluator::Evaluate(value, context);

        if (evaluation.useOffset)
        {
            for (uint64_t i = 0; i < valueSize; i++)
            {
                uint8_t byte = static_cast<uint8_t>((evaluation.offset >> (i * 8)) & 0xFF);
                internalBuffer.push_back(byte);
            }

            Object::Relocation relocation;
            relocation.offsetInSection = context.sectionOffset + offset;
            relocation.addend = evaluation.offset;
            relocation.usedSection = evaluation.usedSection;
            relocation.isExtern = evaluation.isExtern;
            relocation.isSigned = false; // TODO: Check

            relocation.type = Object::Relocation::Type::Absolute;
            switch (valueSize)
            {
                case 1: relocation.size = Object::Relocation::Size::Bit8; break;
                case 2: relocation.size = Object::Relocation::Size::Bit16; break;
                case 3: relocation.size = Object::Relocation::Size::Bit24; break;
                case 4: relocation.size = Object::Relocation::Size::Bit32; break;
                case 8: relocation.size = Object::Relocation::Size::Bit64; break;

                default:
                    throw Exception::InternalError("Invalid size in bits " + std::to_string(valueSize), line, column, file.c_str());
            }

            relocBuffer.push_back(relocation);
        }
        else
        {
            for (uint64_t i = 0; i < valueSize; i++)
            {
                uint8_t byte = static_cast<uint8_t>((evaluation.result >> (i * 8)) & 0xFF);
                internalBuffer.push_back(byte);
            }
        }

        offset += valueSize;
    }
}

bool Assembler::Instructions::Data::optimize()
{
    return false;
}

Assembler::Instructions::Reserved::Reserved(uint64_t size, const AsmProg::Immediate& v, StringPool::String empty, uint64_t l, uint64_t c, StringPool::String f)
    : Instruction(l, c, f), valueSize(size), value(v)
{
    if (value.operands.empty())
        throw Exception::SyntaxError("Missing values after data definition", line, column, file.c_str());

    // TODO: Make more dynamic
    internalBuffer.clear();

    // TODO: ugly
    std::vector<AsmProg::Symbol> emptySymbols;
    std::unordered_map<std::string, uint64_t> emptyLabelMap;
    std::unordered_map<std::string, uint64_t> emptyConstantMap;
    std::unordered_map<std::string, uint64_t> emptySectionStart;
    Evaluator::Context evaluatorContext(
        line, column, file,
        emptySymbols, emptyLabelMap, emptyConstantMap,
        0, 0,
        empty, emptySectionStart
    );

    Evaluator::Result evaluation = Evaluator::Evaluate(value, evaluatorContext);

    if (evaluation.useOffset)
        throw Exception::SemanticError("Can't use relocations for reserved data", line, column, file.c_str());

    bufferSize = valueSize * evaluation.result;

    for (uint64_t i = 0; i < bufferSize; i++)
    {
        internalBuffer.push_back(static_cast<uint8_t>(0));
    }
}

void Assembler::Instructions::Reserved::encode(std::vector<uint8_t>& buffer, std::vector<Object::Relocation>* relocations, Evaluator::Context& context)
{
    (void)context;
    (void)relocations;
    buffer.insert(buffer.end(), internalBuffer.begin(), internalBuffer.end());
}

uint64_t Assembler::Instructions::Reserved::size()
{
    return bufferSize;
}

void Assembler::Instructions::Reserved::evaluate(Evaluator::Context& context)
{
    (void)context;
    // TODO
}

bool Assembler::Instructions::Reserved::optimize()
{
    return false;
}
