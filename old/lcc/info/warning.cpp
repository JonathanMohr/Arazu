#include "warning.hpp"
#include <stdexcept>

void LCC::Warning::Print(std::ostream& os) const
{
    // TODO: Color
    os << '[' << filename.str() << ':' << line << ',' << column << "] ";

    switch (type)
    {
        case Type::CouldNotGetDateAndTime:
            os << "Could not get date and time";
            break;


        
        case Type::ArgumentDoesNotDoAnything:
            os << extra << " does not do anything";
            break;


        
        case Type::NonStandardSyntaxForLineDefinition:
            os << "Non-standard syntax used to specify line and file";
            break;


        case Type::HexEscapeOverflow:
            os << "Hex escape sequence out of range";
            break;

        case Type::OctalEscapeOverflow:
            os << "Octal escape sequence out of range";
            break;


        case Type::Multichar:
            os << "Multi-character character constant";
            break;


        default:
            os << "Invalid warning";
            break;
    }

    os << '\n';
}

void LCC::WarningManager::printAll(std::ostream& os) const
{
    for (const Warning& warning : warnings)
        warning.Print(os);
}

LCC::Exception::Exception(LCC::Exception::Type _type, uint64_t _line, uint64_t _column, const StringPool::String& _filename, std::string _extra)
    : std::runtime_error("Exception occurred"), type(_type), line(_line), column(_column), filename(_filename), extra(std::move(_extra))
{
    // TODO
}

void LCC::Exception::Print(std::ostream& os) const
{
    // TODO: Color
    os << '[' << filename.str() << ':' << line << ',' << column << "] ";

    switch (type)
    {
        case Type::Internal_PPHeaderNameInConvert:
            os << "Internal: Header-Name token cannot be converted";
            break;

        case Type::Internal_InvalidOperatorSpecificKind:
            os << "Internal: Invalid operator specific kind";
            break;


        case Type::BackslashAtEndOfFile:
            os << "Backlash at end of file";
            break;


        case Type::UnexpectedCharacter:
            os << "Unexpected character";
            break;

        case Type::UnterminatedBlockComment:
            os << "Unterminated /* comment";
            break;

        
        case Type::HexConstantNoDigits:
            os << "No hexadecimal digits after hex constant";
            break;

        case Type::InvalidOctalDigit:
            os << "Invalid digit in octal constant";
            break;


        case Type::FloatExponentNoDigits:
            os << "Exponent has no digits";
            break;


        case Type::InvalidIntegerSuffix:
            os << "Invalid suffix on integer constant";
            break;

        case Type::InvalidFloatSuffix:
            os << "Invalid suffix on floating constant";
            break;


        case Type::UnterminatedCharLiteral:
            os << "Missing terminating ' character";
            break;

        case Type::EmptyCharLiteral:
            os << "Empty character constant";
            break;


        case Type::InvalidSourceCharacter:
            os << "Invalid source character";
            break;

        case Type::InvalidEscapeSequence:
            os << "Invalid escape sequence";
            break;

        
        case Type::HexEscapeNoDigits:
            os << "No hexadecimal digits after hex escape";
            break;


        case Type::UnterminatedStringLiteral:
            os << "Missing terminating \" character";
            break;


        
        case Type::SetFileNotString:
            os << "File needs to be a string";
            break;

        
        case Type::UserDefinedError:
            os << "#error: " << extra;
            break;


        case Type::InvalidPreprocessingDirective:
            os << "Invalid preprocessing directive";
            break;


        case Type::MacroNameNotIdentifier:
            os << "Macro name must be an identifier";
            break;

        case Type::MacroParameterNoIdentifier:
            os << "Expected identifier in macro parameter list";
            break;

        case Type::MacroUnterminatedParameterList:
            os << "Missing terminating ) character in macro parameter list";
            break;

        case Type::MacroNoCommaInParameterList:
            os << "Expected comma in macro parameter list";
            break;

        
        case Type::MacroInvalidName:
            os << "Invalid macro name";
            break;

        
        case Type::HashHashAtStartOfReplacementList:
            os << "## cannot appear at the start of a macro replacement list";
            break;

        case Type::HashHashAtEndOfReplacementList:
            os << "## cannot appear at the end of a macro replacement list";
            break;

        case Type::HashHashInvalidConcatenation:
            os << "Concatenation does not give a valid preprocessing token";
            break;

        case Type::HashOperatorNotFollowedByParameter:
            os << "# is not followed by a macro parameter";
            break;


        case Type::MacroUnterminatedInvocation:
            os << "Unterminated argument list invoking macro";
            break;

        case Type::MacroWrongArgumentCount:
            os << "Macro passed the wrong amount of arguments";
            break;

        case Type::MacroEmptyArgument:
            os << "Macro argument count is 0";
            break;

        case Type::MacroRedefinitionIncompatible:
            os << "Incompatible macro redefinition";
            break;


        case Type::TokenAfterPPDirective:
            os << "Extra tokens after directive";
            break;


        case Type::NewlineAfterInclude:
            os << "Missing file after directive";
            break;

        case Type::UnterminatedIncludeDirective:
            os << "Unterminated include directive";
            break;

        case Type::IncludedFileNotFound:
            os << extra << " could not be found";
            break;

        case Type::RecursiveInclude:
            os << "Recursive include";
            break;


        case Type::ConditionalPPDirectiveWithoutIf:
            os << "#" << extra << " directive without #if directive";
            break;

        case Type::ConditionalPPElseAfterElse:
            os << "#else after #else";
            break;

        case Type::UnterminatedConditionalPPDirective:
            os << "Unterminated conditional preprocessor directive";
            break;

        case Type::InvalidPreprocessingExpression:
            os << "Invalid preprocessor expression";
            break;


        case Type::LineNoPositiveInteger:
            os << "#line directive requires a positive integer argument";
            break;

        case Type::LineInvalidFilename:
            os << "Invalid filename for #line directive";
            break;


        case Type::DivisionByZero:
            os << "Division by zero";
            break;


        default:
            os << "Invalid exception";
            break;
    }

    os << '\n';
}
