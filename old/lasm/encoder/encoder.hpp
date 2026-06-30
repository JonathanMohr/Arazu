#pragma once

#include "../asmprog/asmprog.hpp"
#include "../object/object.hpp"

#include "../context.hpp"
#include "architecture/architecture.hpp"
#include <unordered_map>
#include <unordered_set>

namespace Assembler
{
    class Encoder
    {
    public:
        static Object Encode(const AsmProg& asmProg, const Context& context);

    private:
        struct FContext
        {
            FContext(AsmProg ap, const Context& c, Architecture a, BitMode b)
                : asmProg(std::move(ap)), context(c), architecture(a), bits(b) {}

            AsmProg asmProg;

            std::unordered_map<std::string, uint64_t> sectionStarts;

            const Context& context;
            Architecture architecture;
            BitMode bits;
        };

        static void Calculate(FContext& context);
        static void Evaluate(FContext& context);
        static bool Optimize(FContext& context);

        static void resolveConstants(FContext& context, bool withPos);
        static bool resolveConstantWithPos(FContext& context, AsmProg::Constant& constant, std::unordered_set<std::string>& visited);
        static bool resolveConstantWithoutPos(FContext& context, AsmProg::Constant& constant, std::unordered_set<std::string>& visited);
    };
}
