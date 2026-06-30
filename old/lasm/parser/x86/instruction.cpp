#include "parser.hpp"

#include <util/string.hpp>

#include "../../instruction/x86.hpp"

std::tuple<bool, bool, std::optional<Assembler::AsmProg::SectionEntry>> Assembler::x86::Parser::parseInstruction(AsmProg& asmProg, const Tokens& tokens, uint64_t& i)
{
    (void)asmProg;

    const Tokens::Token& token = tokens[i];
    const std::string lowerValue = toLower(token.value);

    auto it = instructionMap.find(lowerValue);
    if (it == instructionMap.end())
        return {false, false, std::nullopt};
    
    switch (it->second)
    {
        case Instruction::AAA: case Instruction::AAS:
        case Instruction::AAD: case Instruction::AAM:
            return std::make_tuple(true, false, parseASCIIInstruction(it->second, tokens, i));

        case Instruction::NOP: case Instruction::HLT:
        case Instruction::JMP: case Instruction::JE: case Instruction::JNE:
        case Instruction::JG: case Instruction::JGE: case Instruction::JL:
        case Instruction::JLE: case Instruction::JA: case Instruction::JAE:
        case Instruction::JB: case Instruction::JBE: case Instruction::JO:
        case Instruction::JNO: case Instruction::JS: case Instruction::JNS:
        case Instruction::JP: case Instruction::JNP: case Instruction::JC:
        case Instruction::JNC:
        case Instruction::CALL: case Instruction::RET:
            return std::make_tuple(true, false, parseControlInstruction(it->second, tokens, i));

        case Instruction::PUSH: case Instruction::POP:
        case Instruction::PUSHA: case Instruction::POPA:
        case Instruction::PUSHAD: case Instruction::POPAD:
        case Instruction::PUSHF: case Instruction::POPF:
        case Instruction::PUSHFD: case Instruction::POPFD:
        case Instruction::PUSHFQ: case Instruction::POPFQ:
            return std::make_tuple(true, false, parseStackInstruction(it->second, tokens, i));

        case Instruction::CLC: case Instruction::STC: case Instruction::CMC:
        case Instruction::CLD: case Instruction::STD:
        case Instruction::CLI: case Instruction::STI:
        case Instruction::LAHF: case Instruction::SAHF:
            return std::make_tuple(true, false, parseFlagInstruction(it->second, tokens, i));

        case Instruction::INT:
        case Instruction::INT3: case Instruction::INTO:
        case Instruction::INT1: case Instruction::IRET:
        case Instruction::IRETQ: case Instruction::IRETD:
        case Instruction::SYSCALL: case Instruction::SYSRET:
        case Instruction::SYSENTER: case Instruction::SYSEXIT:
            return std::make_tuple(true, false, parseInterruptInstruction(it->second, tokens, i));

        case Instruction::ADD: case Instruction::ADC:
        case Instruction::SUB: case Instruction::SBB:
        case Instruction::CMP: case Instruction::TEST:
        case Instruction::AND: case Instruction::OR:
        case Instruction::XOR:
        case Instruction::ADCX: case Instruction::ADOX:
        case Instruction::MUL: case Instruction::IMUL:
        case Instruction::DIV: case Instruction::IDIV:
        case Instruction::SHL: case Instruction::SHR:
        case Instruction::SAL: case Instruction::SAR:
        case Instruction::ROL: case Instruction::ROR:
        case Instruction::RCL: case Instruction::RCR:
        case Instruction::NOT: case Instruction::NEG:
        case Instruction::INC: case Instruction::DEC:
            return std::make_tuple(true, false, parseALUInstruction(it->second, tokens, i));

        case Instruction::MOV:
            return std::make_tuple(true, false, parseDataInstruction(it->second, tokens, i));

        default:
            throw Exception::InternalError("Invalid instruction", token.line, token.column, token.file.c_str());
    }
}
