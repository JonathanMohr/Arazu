#pragma once

#include <Architecture.hpp>
#include <StringPool.hpp>
#include <cstdint>
#include <vector>
#include <variant>
#include <memory>

namespace Assembler
{
    class Instruction;

    class AsmProg
    {
    public:
        struct Integer
        {
            uint64_t value;
        };

        struct Operator
        {
            char op;
        };

        struct String
        {
            StringPool::String value;
        };

        struct Position
        {
            bool sectionPos;
        };

        struct Immediate
        {
            struct Operand
            {
                inline Operand(const Integer& integer) : operand(integer) {}
                inline Operand(const Operator& op) : operand(op) {}
                inline Operand(const String& str) : operand(str) {}
                inline Operand(const Position& currentPosition) : operand(currentPosition) {}

                inline bool isInteger()  const { return std::holds_alternative<Integer>(operand); }
                inline bool isOperator() const { return std::holds_alternative<Operator>(operand); }
                inline bool isString()   const { return std::holds_alternative<String>(operand); }
                inline bool isPosition() const { return std::holds_alternative<Position>(operand); }

                inline Integer  getInteger()  const { return std::get<Integer>(operand); }
                inline Operator getOperator() const { return std::get<Operator>(operand); }
                inline String   getString()   const { return std::get<String>(operand); }
                inline Position getPosition() const { return std::get<Position>(operand); }

                std::variant<Integer, Operator, String, Position> operand;
            };

            std::vector<Operand> operands;
        };

        using Register = uint64_t;

        struct Memory
        {
            Immediate displacement;

            Register reg1;
            Immediate scale1;

            Register reg2;
            Immediate scale2;

            static constexpr uint64_t POINTER_SIZE_NONE = 0;
            uint64_t pointerSize;

            enum Used : uint8_t
            {
                UNSURE,
                REG1,
                REG2
            } used;

            bool useReg2 = false;
            bool useReg1 = false;
            bool useDisplacement = false;
        };

        struct Operand
        {
            inline Operand(const Register& reg)        : operand(reg) {}
            inline Operand(const Memory& memory)       : operand(memory) {}
            inline Operand(const Immediate& immediate) : operand(immediate) {}

            inline bool isRegister()  const { return std::holds_alternative<Register>(operand); }
            inline bool isMemory()    const { return std::holds_alternative<Memory>(operand); }
            inline bool isImmediate() const { return std::holds_alternative<Immediate>(operand); }

            inline Register  getRegister()  const { return std::get<Register>(operand); }
            inline Memory    getMemory()    const { return std::get<Memory>(operand); }
            inline Immediate getImmediate() const { return std::get<Immediate>(operand); }

            std::variant<Register, Memory, Immediate> operand;
        };

        struct Label
        {
            StringPool::String name;
            StringPool::String section;
            uint64_t offset = 0;

            uint64_t line;
            uint64_t column;
            StringPool::String file;

            bool isGlobal;
            bool isExtern;

            bool resolved = false;
            bool externUsed = false;
        };

        struct Constant
        {
            enum class HasPos : uint8_t
            {
                UNKNOWN,
                TRUE,
                FALSE
            };

            StringPool::String name;
            StringPool::String section;
            Immediate expression;
            int64_t value;

            uint64_t line;
            uint64_t column;
            StringPool::String file;

            HasPos hasPos = HasPos::UNKNOWN;
            bool useOffset = false;

            int64_t off;
            StringPool::String usedSection;

            uint64_t offset;
            uint64_t bytesWritten;

            bool isGlobal;
            bool resolved = false;
            bool relocationPossible = false;
        };

        struct Symbol
        {
            inline Symbol(const Label& l)    : symbol(l) {}
            inline Symbol(const Constant& c) : symbol(c) {}

            inline bool isLabel()    const { return std::holds_alternative<Label>(symbol); }
            inline bool isConstant() const { return std::holds_alternative<Constant>(symbol); }

            inline Label&    getLabel()    { return std::get<Label>(symbol); }
            inline Constant& getConstant() { return std::get<Constant>(symbol); }

            inline const Label&    getLabel()    const { return std::get<Label>(symbol); }
            inline const Constant& getConstant() const { return std::get<Constant>(symbol); }

            std::variant<Label, Constant> symbol;
        };

        struct SectionEntry
        {
            struct Label
            {
                inline Label(uint64_t idx) : index(idx) {}
                uint64_t index;
            };

            struct Constant
            {
                inline Constant(uint64_t idx) : index(idx) {}
                uint64_t index;
            };

            inline SectionEntry(std::shared_ptr<Instruction> instruction) : entry(std::move(instruction)) {}
            inline SectionEntry(const Label& label)                       : entry(label) {}
            inline SectionEntry(const Constant& constant)                 : entry(constant) {}
            ~SectionEntry() = default;

            inline bool isInstruction() const { return std::holds_alternative<std::shared_ptr<Instruction>>(entry); }
            inline bool isLabel()       const { return std::holds_alternative<Label>(entry); }
            inline bool isConstant()    const { return std::holds_alternative<Constant>(entry); }

            inline Instruction* getInstruction() const { return std::get<std::shared_ptr<Instruction>>(entry).get(); }
            inline Label        getLabel()       const { return std::get<Label>(entry); }
            inline Constant     getConstant()    const { return std::get<Constant>(entry); }

            std::variant<std::shared_ptr<Instruction>, Label, Constant> entry;
        };

        struct Section
        {
            StringPool::String name;
            uint64_t align = 1;
            std::vector<SectionEntry> entries;

            bool isInitialized = false;
            
            bool isAllocatable = false;
            bool isExecutable = false;
            bool isWritable = false;
        };

    private:
        std::vector<Symbol> symbols;

        std::unordered_map<std::string, uint64_t> labelMap;
        std::unordered_map<std::string, uint64_t> constantMap;

        // bool in pair:
        //  true: label
        //  false: constant
        std::unordered_map<std::string, std::pair<bool, uint64_t>> symbolMap;

        std::string origin;

        Architecture architecture;
        BitMode bits;

        std::vector<Section> sections;

        friend class Parser;
        friend class Encoder;
    };
}
