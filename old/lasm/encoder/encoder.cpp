#include "encoder.hpp"

#include "../instruction/instruction.hpp"
#include "../instruction/evaluate.hpp"

Assembler::Object Assembler::Encoder::Encode(const AsmProg& asmProg, const Context& context)
{
    FContext fcontext(asmProg, context, asmProg.architecture, asmProg.bits);

    resolveConstants(fcontext, false);

    bool changed = true;
    while (changed)
    {
        Calculate(fcontext);
        Evaluate(fcontext);
        changed = Optimize(fcontext);
    }

    Object object;
    object.architecture = fcontext.asmProg.architecture;
    object.bits = fcontext.asmProg.bits;

    std::unordered_map<std::string, uint64_t> sectionMap;

    uint64_t bytesWritten = 0;
    for (const AsmProg::Section& s : fcontext.asmProg.sections)
    {
        Object::Section section;
        section.name = s.name;
        section.align = s.align;

        section.flags = 0;
        if (s.isInitialized)
            section.type = Object::Section::Type::Initialized;
        else
            section.type = Object::Section::Type::Uninitialized;

        if (s.isAllocatable)
            section.flags |= Object::Section::FlagsAllocatable;
        if (s.isExecutable)
            section.flags |= Object::Section::FlagsExecutable;
        if (s.isWritable)
            section.flags |= Object::Section::FlagsWritable;

        section.size = 0;

        Evaluator::Context evaluatorContext(
            0, 0, fcontext.context.stringPool->empty(),
            fcontext.asmProg.symbols, fcontext.asmProg.labelMap, fcontext.asmProg.constantMap,
            0, 0, 
            section.name, fcontext.sectionStarts
        );

        for (const AsmProg::SectionEntry& entry : s.entries)
        {
            if (entry.isInstruction())
            {
                Instruction* instruction = entry.getInstruction();

                evaluatorContext.line = instruction->getLine();
                evaluatorContext.column = instruction->getColumn();
                evaluatorContext.file = instruction->getFile();

                evaluatorContext.bytesWritten = bytesWritten;
                evaluatorContext.sectionOffset = section.size;

                instruction->encode(section.buffer, &section.relocations, evaluatorContext);

                const uint64_t size = instruction->size();
                section.size += size;
                bytesWritten += size;
            }
            else if (entry.isLabel() || entry.isConstant())
            {
                // Ignore
            }
            else
            {
                throw Exception::InternalError("Couldn't find logic for Encoder::SectionEntry", 0, 0);
            }
        }

        sectionMap[s.name.str()] = object.sections.size();
        object.sections.push_back(std::move(section));
    }

    for (const AsmProg::Symbol& symbol : fcontext.asmProg.symbols)
    {
        if (symbol.isLabel())
        {
            const AsmProg::Label& label = symbol.getLabel();

            // Ignore unused extern labels
            if (label.isExtern && !label.externUsed) continue;

            Object::Symbol sym;
            sym.name = label.name;
            sym.value = label.offset;

            if (label.isExtern)
            {
                sym.section = std::nullopt;
                sym.absolute = false;
                sym.isGlobal = true;
            }
            else
            {
                sym.section = label.section;
                sym.absolute = true;
                sym.isGlobal = label.isGlobal;
            }

            sym.line = label.line;
            sym.column = label.column;
            sym.file = label.file;

            object.symbols.push_back(sym);
        }
        else
        {
            const AsmProg::Constant& constant = symbol.getConstant();

            Object::Symbol sym;
            sym.name = constant.name;
            sym.absolute = true;
            
            if (constant.useOffset)
            {
                sym.value = static_cast<uint64_t>(constant.off);
                sym.section = constant.usedSection;
            }
            else
            {
                sym.value = static_cast<uint64_t>(constant.value);
                sym.section = std::nullopt;
            }

            sym.isGlobal = constant.isGlobal;

            sym.line = constant.line;
            sym.column = constant.column;
            sym.file = constant.file;

            object.symbols.push_back(sym);
        }
    }
    
    return object;
}

void Assembler::Encoder::Calculate(Assembler::Encoder::FContext& context)
{
    context.sectionStarts.clear();

    uint64_t bytesWritten = 0;
    for (const AsmProg::Section& section : context.asmProg.sections)
    {
        context.sectionStarts[section.name.str()] = bytesWritten;
        uint64_t sectionOffset = 0;
        
        for (const AsmProg::SectionEntry& entry : section.entries)
        {
            if (entry.isInstruction())
            {
                const uint64_t size = entry.getInstruction()->size();
                sectionOffset += size;
                bytesWritten += size;
            }
            else if (entry.isLabel())
            {
                AsmProg::Label& label = context.asmProg.symbols[entry.getLabel().index].getLabel();
                label.offset = sectionOffset;
                label.resolved = true;
            }
            else if (entry.isConstant())
            {
                AsmProg::Constant& constant = context.asmProg.symbols[entry.getConstant().index].getConstant();
                constant.offset = sectionOffset;
                constant.bytesWritten = bytesWritten;
            }
            else
            {
                // TODO: Add line, column and file
                throw Exception::InternalError("Invalid section entry", 0, 0);
            }
        }
    }

    resolveConstants(context, true);
}

void Assembler::Encoder::Evaluate(Assembler::Encoder::FContext& context)
{
    uint64_t bytesWritten = 0;
    for (const AsmProg::Section& section : context.asmProg.sections)
    {
        uint64_t sectionOffset = 0;

        Evaluator::Context evaluatorContext(
            0, 0, context.context.stringPool->empty(),
            context.asmProg.symbols, context.asmProg.labelMap, context.asmProg.constantMap,
            0, 0, 
            section.name, context.sectionStarts
        );
        
        for (const AsmProg::SectionEntry& entry : section.entries)
        {
            if (entry.isInstruction())
            {
                Instruction* instruction = entry.getInstruction();

                evaluatorContext.line = instruction->getLine();
                evaluatorContext.column = instruction->getColumn();
                evaluatorContext.file = instruction->getFile();

                evaluatorContext.bytesWritten = bytesWritten;
                evaluatorContext.sectionOffset = sectionOffset;

                instruction->evaluate(evaluatorContext);

                const uint64_t size = instruction->size();
                sectionOffset += size;
                bytesWritten += size;
            }
            else if (entry.isLabel() || entry.isConstant())
            {
                // Ignore
            }
            else
            {
                // TODO: Add line, column and file
                throw Exception::InternalError("Invalid section entry", 0, 0);
            }
        }
    }
}

bool Assembler::Encoder::Optimize(Assembler::Encoder::FContext& context)
{
    bool changed = false;

    for (const AsmProg::Section& section : context.asmProg.sections)
    {
        for (const AsmProg::SectionEntry& entry : section.entries)
        {
            if (entry.isInstruction())
            {
                Instruction* instruction = entry.getInstruction();
                if (instruction->optimize())
                    changed = true;
            }
            else if (entry.isLabel() || entry.isConstant())
            {
                // Ignore
            }
            else
            {
                // TODO: Add line, column and file
                throw Exception::InternalError("Invalid section entry", 0, 0);
            }
        }
    }

    return changed;
}
