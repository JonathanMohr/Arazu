#pragma once

#include <cstdint>
#include <ostream>
#include <optional>
#include <memory>

#include "../context.hpp"
#include "../asmprog/asmprog.hpp"

namespace Assembler
{
    class Tokens
    {
    public:
        enum class Type : uint8_t
        {
            Token,
            String,
            Character,
            Operator,
            Comma,
            Punctuation,
            Bracket,
            EOL,
            EOFile
        };

        struct Token
        {
            Token(Type t, const StringPool::String& v, uint64_t l, uint64_t c, const StringPool::String& f, uint64_t s)
                : type(t), value(v), line(l), column(c), file(f), spaceCount(s) {}

            std::string what() const noexcept;

            Type type;
            StringPool::String value;

            uint64_t line;
            uint64_t column;
            StringPool::String file;

            uint64_t spaceCount;
        };

        void Print(std::ostream* out) const;
        inline void Print(std::ostream& out) const { Print(&out); }

        using const_iterator = std::vector<Token>::const_iterator;
        inline const_iterator begin() const { return tokens.begin(); }
        inline const_iterator end()   const { return tokens.end(); }
        inline const Token& operator[](uint64_t i) const { return tokens[i]; }

        inline uint64_t size () const { return tokens.size(); }
        inline bool empty()     const { return tokens.empty(); }

    private:
        std::vector<Token> tokens;

        inline void addToken(Type type, const StringPool::String& value, uint64_t line, uint64_t column, const StringPool::String& file, uint64_t spaceCount)
        {
            tokens.emplace_back(type, value, line, column, file, spaceCount);
        }

        friend class Tokenizer;
    };

    class Tokenizer
    {
    public:
        static Tokens Tokenize(const Context& context, const std::vector<std::string>& filenames);

    private:
        static void Tokenize(const Context& context, const char* filename, Tokens& tokens);
    };

    class Parser
    {
    public:
        Parser(const Context& _context, Architecture _arch, BitMode _bits);
        virtual ~Parser() = default;

        AsmProg Parse(const Tokens& tokens);

        static std::unique_ptr<Parser> Get(const Context& c, Architecture a, BitMode b);

    protected:
        inline bool isLabelGlobal(const StringPool::String& label) { return globals.find(label.str()) != globals.end(); }
        inline void SetOrigin(AsmProg& asmProg, const std::string& origin) { asmProg.origin = origin; }

        uint64_t addLabel(AsmProg& asmProg, const AsmProg::Label& label);
        uint64_t addConstant(AsmProg& asmProg, const AsmProg::Constant& constant);

        virtual void handleLabels(AsmProg& asmProg, const Tokens& tokens) = 0;

        AsmProg::Section* getSection(AsmProg& asmProg, const StringPool::String& name);

        virtual std::tuple<bool, bool, std::optional<AsmProg::SectionEntry>> parseDirective(AsmProg& asmProg, const Tokens& tokens, uint64_t& i) = 0;
        virtual std::tuple<bool, bool, std::optional<AsmProg::SectionEntry>> parseSymbol(AsmProg& asmProg, const Tokens& tokens, uint64_t& i) = 0;
        virtual std::tuple<bool, bool, std::optional<AsmProg::SectionEntry>> parsePseudo(AsmProg& asmProg, const Tokens& tokens, uint64_t& i) = 0;
        virtual std::tuple<bool, bool, std::optional<AsmProg::SectionEntry>> parseInstruction(AsmProg& asmProg, const Tokens& tokens, uint64_t& i) = 0;

        BitMode currentBitMode;

        std::vector<AsmProg::SectionEntry>* currentEntryVector;
        StringPool::String currentSectionName;
        StringPool::String lastMainLabel;

        std::unordered_map<std::string, uint64_t> sectionMap;

        struct GlobalInformation {
            GlobalInformation(uint64_t l, uint64_t c, const StringPool::String& f)
                : line(l), column(c), file(f) {}

            uint64_t line;
            uint64_t column;
            StringPool::String file;

            bool defined = false;
        };

        std::unordered_map<std::string, GlobalInformation> globals;

        const Context& context;
        Architecture arch;
        BitMode bits;
    };
}
