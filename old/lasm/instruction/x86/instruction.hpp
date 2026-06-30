#pragma once

#include "../instruction.hpp"
#include "../x86.hpp"

namespace Assembler
{
    namespace x86
    {
        class D_Instruction : public Assembler::Instruction
        {
        public:
            D_Instruction(BitMode bitmode, uint64_t l, uint64_t c, StringPool::String f);
            virtual ~D_Instruction() = default;

            void encode(std::vector<uint8_t>& buffer, std::vector<Object::Relocation>* relocations, Evaluator::Context& context) override final;
            uint64_t size() override final;
            void evaluate(Evaluator::Context& context) override final;
            bool optimize() override final;

        protected:
            void checkSize(uint64_t size);
            void checkReg(const AsmProg::Register& reg);
            void checkRegFor8BitREX(const AsmProg::Register& reg);

            uint64_t parseMemory(const AsmProg::Memory& memory);
            uint64_t parseMemory(const AsmProg::Memory& memory, uint64_t implicitSize);
            uint64_t parseRegister(const AsmProg::Register& reg, bool forceGPR, bool isRM);

            uint64_t getDisplacementOffset();
            uint64_t getImmediateOffset();

            virtual void encodeS(std::vector<uint8_t>& buffer, std::vector<Object::Relocation>* relocations) { (void)buffer; (void)relocations; }
            virtual uint64_t sizeS() { return 0; }
            virtual void evaluateS(Evaluator::Context& context) { (void)context; }
            virtual bool optimizeS() { return false; }

            BitMode bits;

            bool use16BitPrefix = false;
            bool use16BitAddressPrefix = false;

            bool use66OpcodeOverride = false;
            bool useF3OpcodeOverride = false;

            enum class AddressMode : uint8_t {
                Bits16,
                Bits32,
                Bits64
            } addressMode;

            struct REX {
                bool use = false;

                bool w = false;
                bool r = false;
                bool x = false;
                bool b = false;
            } rex;

            enum class OpcodeEscape : uint8_t {
                NONE,
                TWO_BYTE,
                THREE_BYTE_38,
                THREE_BYTE_3A
            } opcodeEscape = OpcodeEscape::NONE;

            uint8_t opcode;

            struct ModRM {
                bool use = false;

                Mod mod = Mod::REGISTER;
                uint8_t reg = 0;
                uint8_t rm = 0;
            } modrm;

            struct SIB {
                bool use = false;

                uint8_t index = 0;
                Scale scale = Scale::x1;
            } sib;

            struct Displacement {
                bool use = false;
                bool is_signed = false;
                bool is_short = false;

                bool can_optimize = true;
                bool direct_addressing = false;

                int64_t value;
                AsmProg::Immediate immediate;

                bool needsRelocation = false;
                StringPool::String relocationUsedSection;
                bool relocationIsExtern;
            } displacement;

            struct Immediate {
                bool use = false;
                bool is_signed = false;

                uint16_t sizeInBits;

                uint64_t value;
                AsmProg::Immediate immediate;

                bool needsRelocation = false;
                StringPool::String relocationUsedSection;
                bool relocationIsExtern;

                bool ripRelative = false;
            } immediate;
        };
    }
}
