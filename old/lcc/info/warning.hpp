#pragma once

#include <StringPool.hpp>
#include <ostream>
#include <stdexcept>

namespace LCC
{
    class Warning
    {
    public:
        enum class Type : uint8_t
        {
            CouldNotGetDateAndTime,

            // Arguments
            ArgumentDoesNotDoAnything,

            // Tokenizer
            NonStandardSyntaxForLineDefinition,

            //  Constants
            HexEscapeOverflow,
            OctalEscapeOverflow,

            Multichar,
        };

        Warning(Type _type, uint64_t _line, uint64_t _column, const StringPool::String& _filename, std::string _extra = "")
            : type(_type), line(_line), column(_column), filename(_filename), extra(std::move(_extra)) {}

        ~Warning() = default;

        void Print(std::ostream& os) const;

    private:
        Type type;
        uint64_t line;
        uint64_t column;
        StringPool::String filename;
        std::string extra;
    };

    class WarningManager
    {
    public:
        WarningManager() {}
        ~WarningManager() = default;

        inline bool hasWarnings() const { return !warnings.empty(); };
        inline void add(const Warning& w) { warnings.push_back(w); }
        inline void clear() { warnings.clear(); }

        void printAll(std::ostream& os) const;

    private:
        std::vector<Warning> warnings;
    };

    class Exception : public std::runtime_error
    {
    public:
        enum class Type : uint8_t
        {
            // Tokenizer

            Internal_PPHeaderNameInConvert,
            Internal_InvalidOperatorSpecificKind,

            BackslashAtEndOfFile,

            UnexpectedCharacter,

            UnterminatedBlockComment,

            //  Constants
            HexConstantNoDigits,
            InvalidOctalDigit,

            FloatExponentNoDigits,

            InvalidIntegerSuffix,
            InvalidFloatSuffix,

            UnterminatedCharLiteral,
            EmptyCharLiteral,

            InvalidSourceCharacter,
            InvalidEscapeSequence,

            HexEscapeNoDigits,

            // String
            UnterminatedStringLiteral,


            // Preprocessor
            SetFileNotString,


            UserDefinedError,

            InvalidPreprocessingDirective,

            MacroNameNotIdentifier,
            MacroParameterNoIdentifier,
            MacroUnterminatedParameterList,
            MacroNoCommaInParameterList,

            MacroInvalidName,

            HashHashAtStartOfReplacementList,
            HashHashAtEndOfReplacementList,
            HashHashInvalidConcatenation,
            HashOperatorNotFollowedByParameter,

            MacroUnterminatedInvocation,
            MacroWrongArgumentCount,
            MacroEmptyArgument,

            MacroRedefinitionIncompatible,

            TokenAfterPPDirective,

            NewlineAfterInclude,
            UnterminatedIncludeDirective,
            IncludedFileNotFound,
            RecursiveInclude,

            ConditionalPPDirectiveWithoutIf,
            ConditionalPPElseAfterElse,
            UnterminatedConditionalPPDirective,
            InvalidPreprocessingExpression,

            LineNoPositiveInteger,
            LineInvalidFilename,

            DivisionByZero
        };

        Exception(Type _type, uint64_t _line, uint64_t _column, const StringPool::String& _filename, std::string _extra = "");

        ~Exception() = default;

        void Print(std::ostream& os) const;

    private:
        Type type;
        uint64_t line;
        uint64_t column;
        StringPool::String filename;
        std::string extra;
    };
}
