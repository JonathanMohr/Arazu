#include "encoder.hpp"

#include "../instruction/evaluate.hpp"

void Assembler::Encoder::resolveConstants(Assembler::Encoder::FContext& context, bool withPos)
{
    std::unordered_set<std::string> visited;

    for (const auto& [name, index] : context.asmProg.constantMap)
    {
        AsmProg::Constant& constant = context.asmProg.symbols[index].getConstant();
        if (!constant.resolved)
        {
            if (withPos)
                (void)resolveConstantWithPos(context, constant, visited);
            else
                (void)resolveConstantWithoutPos(context, constant, visited);
        }
    }
}

bool Assembler::Encoder::resolveConstantWithoutPos(Assembler::Encoder::FContext& context, AsmProg::Constant& constant, std::unordered_set<std::string>& visited)
{
    if (constant.resolved) return true;
    if (constant.hasPos == AsmProg::Constant::HasPos::TRUE) return false;

    if (visited.find(constant.name.str()) != visited.end())
        throw Exception::SemanticError("Circular dependency at" + constant.name.str(), constant.line, constant.column, constant.file.c_str());
    visited.insert(constant.name.str());

    std::vector<StringPool::String> deps;
    for (const AsmProg::Immediate::Operand& operand : constant.expression.operands)
    {
        if (operand.isString())
        {
            deps.push_back(operand.getString().value);
        }
        else if (operand.isPosition())
        {
            constant.hasPos = AsmProg::Constant::HasPos::TRUE;
            visited.erase(constant.name.str());
            return false;
        }
    }

    for (const StringPool::String& dep : deps)
    {
        if (context.asmProg.labelMap.find(dep.str()) != context.asmProg.labelMap.end())
        {
            constant.hasPos = AsmProg::Constant::HasPos::TRUE;
            visited.erase(constant.name.str());
            return false;
        }

        auto it = context.asmProg.constantMap.find(dep.str());
        if (it == context.asmProg.constantMap.end())
            throw Exception::SemanticError("Invalid string '" + dep.str() + "'", constant.line, constant.column, constant.file.c_str());
        
        if (!resolveConstantWithoutPos(context, context.asmProg.symbols[it->second].getConstant(), visited))
        {
            constant.hasPos = AsmProg::Constant::HasPos::TRUE;
            visited.erase(constant.name.str());
            return false;
        }
    }

    // TODO: ugly
    Evaluator::Context evaluatorContext(
        constant.line, constant.column, constant.file,
        context.asmProg.symbols, context.asmProg.labelMap, context.asmProg.constantMap,
        0, 0,
        context.context.stringPool->empty(), context.sectionStarts
    );

    Evaluator::Result evaluation = Evaluator::Evaluate(constant.expression, evaluatorContext);
    if (evaluation.relocationPossible) constant.relocationPossible = true;
    if (evaluation.relocationPossible && evaluation.useOffset)
    {
        constant.useOffset = true;
        constant.off = evaluation.offset;
        constant.usedSection = evaluation.usedSection;
    }
    else
    {
        const uint64_t value = evaluation.result;

        // TODO: Overflow
        constant.value = static_cast<int64_t>(value);
    }
    constant.resolved = true;
    
    constant.hasPos = AsmProg::Constant::HasPos::FALSE;
    visited.erase(constant.name.str());
    return true;
}

bool Assembler::Encoder::resolveConstantWithPos(Assembler::Encoder::FContext& context, AsmProg::Constant& constant, std::unordered_set<std::string>& visited)
{
    if (constant.resolved) return true;

    if (visited.find(constant.name.str()) != visited.end())
        throw Exception::SemanticError("Circular dependency at" + constant.name.str(), constant.line, constant.column, constant.file.c_str());
    visited.insert(constant.name.str());

    std::vector<StringPool::String> deps;
    for (const AsmProg::Immediate::Operand& operand : constant.expression.operands)
    {
        if (operand.isString())
        {
            deps.push_back(operand.getString().value);
        }
    }

    for (const StringPool::String& dep : deps)
    {
        if (context.asmProg.labelMap.find(dep.str()) != context.asmProg.labelMap.end())
            continue;

        auto it = context.asmProg.constantMap.find(dep.str());
        if (it == context.asmProg.constantMap.end())
            throw Exception::SemanticError("Invalid string '" + dep.str() + "'", constant.line, constant.column, constant.file.c_str());

        if (!resolveConstantWithPos(context, context.asmProg.symbols[it->second].getConstant(), visited))
            throw Exception::InternalError("Could not resolve constant '" + dep.str() + "'", constant.line, constant.column, constant.file.c_str());
    }

    Evaluator::Context evaluatorContext(
        constant.line, constant.column, constant.file,
        context.asmProg.symbols, context.asmProg.labelMap, context.asmProg.constantMap,
        constant.bytesWritten, constant.offset,
        constant.section, context.sectionStarts
    );

    Evaluator::Result evaluation = Evaluator::Evaluate(constant.expression, evaluatorContext);
    if (evaluation.relocationPossible) constant.relocationPossible = true;
    if (evaluation.relocationPossible && evaluation.useOffset)
    {
        constant.useOffset = true;
        constant.off = evaluation.offset;
        constant.usedSection = evaluation.usedSection;
    }
    else
    {
        const uint64_t value = evaluation.result;

        // TODO: Overflow
        constant.value = static_cast<int64_t>(value);
    }
    constant.resolved = true;
    
    constant.hasPos = AsmProg::Constant::HasPos::TRUE;
    visited.erase(constant.name.str());
    return true;
}
