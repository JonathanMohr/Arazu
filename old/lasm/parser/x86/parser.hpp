#include "../parser.hpp"

#include "../../instruction/x86.hpp"

namespace Assembler
{
    namespace x86
    {
        class Parser : public ::Assembler::Parser
        {
        public:
            Parser(const Context& _context, Architecture _arch, BitMode _bits)
                : ::Assembler::Parser(_context, _arch, _bits) {}
            ~Parser() override = default;

        private:
            void handleLabels(AsmProg& asmProg, const Tokens& tokens) override;

            AsmProg::Immediate::Operand getImmediateOperand(const Tokens::Token& token);

            std::optional<AsmProg::Register> parseRegister(const Tokens& tokens, uint64_t& i);
            std::optional<AsmProg::Memory> parseMemory(const Tokens& tokens, uint64_t& i);
            std::optional<AsmProg::Immediate> parseImmediate(const Tokens& tokens, uint64_t& i);

            std::tuple<bool, bool, std::optional<AsmProg::SectionEntry>> parseDirective(AsmProg& asmProg, const Tokens& tokens, uint64_t& i) override;
            std::tuple<bool, bool, std::optional<AsmProg::SectionEntry>> parseSymbol(AsmProg& asmProg, const Tokens& tokens, uint64_t& i) override;
            std::tuple<bool, bool, std::optional<AsmProg::SectionEntry>> parsePseudo(AsmProg& asmProg, const Tokens& tokens, uint64_t& i) override;
            std::tuple<bool, bool, std::optional<AsmProg::SectionEntry>> parseInstruction(AsmProg& asmProg, const Tokens& tokens, uint64_t& i) override;

            AsmProg::SectionEntry parseASCIIInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i);
            AsmProg::SectionEntry parseControlInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i);
            AsmProg::SectionEntry parseStackInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i);
            AsmProg::SectionEntry parseFlagInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i);
            AsmProg::SectionEntry parseInterruptInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i);
            AsmProg::SectionEntry parseALUInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i);
            AsmProg::SectionEntry parseDataInstruction(Instruction instruction, const Tokens& tokens, uint64_t& i);
        };
    }
}
