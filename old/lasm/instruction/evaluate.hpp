#pragma once

#include "../asmprog/asmprog.hpp"
#include <StringPool.hpp>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace Assembler
{
    namespace Evaluator
    {
        struct Context
        {
            Context(
                uint64_t l, uint64_t c, StringPool::String f,
                std::vector<AsmProg::Symbol>& s,
                const std::unordered_map<std::string, uint64_t>& lm,
                const std::unordered_map<std::string, uint64_t>& cm,
                uint64_t bw,
                uint64_t so,
                StringPool::String curS,
                const std::unordered_map<std::string, uint64_t>& ss
            ) : line(l), column(c), file(f), symbols(s), labelMap(lm),
                constantMap(cm), bytesWritten(bw), sectionOffset(so),
                currentSection(curS), sectionStarts(ss) {}

            uint64_t line;
            uint64_t column;
            StringPool::String file;

            std::vector<AsmProg::Symbol>& symbols;
            const std::unordered_map<std::string, uint64_t>& labelMap;
            const std::unordered_map<std::string, uint64_t>& constantMap;

            uint64_t bytesWritten;
            uint64_t sectionOffset;
            StringPool::String currentSection;

            const std::unordered_map<std::string, uint64_t>& sectionStarts;
        };

        struct Result
        {
            uint64_t result;

            int64_t offset;
            StringPool::String usedSection;
            bool useOffset;
            bool relocationPossible;
            bool isExtern;
        };

        Result Evaluate(const AsmProg::Immediate& immediate, Evaluator::Context& context, bool pcRelative, uint64_t pcExtra);
        inline Result Evaluate(const AsmProg::Immediate& immediate, Evaluator::Context& context) { return Evaluate(immediate, context, false, 0); }
    }

    namespace ShuntingYard
    {
        struct Token
        {
            enum class Type : uint8_t { Number, Operator, Position };
            Type type;

            uint64_t number;
            char op;
            uint64_t offset;
            bool negative = false;

            Token(uint64_t n) : type(Type::Number), number(n) {}
            Token(char o) : type(Type::Operator), op(o) {}
            Token() {}
        };

        struct PreparedTokens {
            std::vector<Token> tokens;

            StringPool::String usedSection;
            bool relocationPossible;
            bool useSection = false;
            bool isExtern = false;
        };

        PreparedTokens prepareTokens(const AsmProg::Immediate& immediate, Evaluator::Context& context);
        uint64_t evaluate(const PreparedTokens& preparedTokens, uint64_t offset, uint64_t line, uint64_t column, const StringPool::String& file);
    }
}
