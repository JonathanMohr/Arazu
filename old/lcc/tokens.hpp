#pragma once

#include <StringPool.hpp>
#include <optional>
#include <vector>

#include "context.hpp"

namespace C
{
    class PPTokens_Old
    {
    public:
        PPTokens_Old() {}
        ~PPTokens_Old() = default;

        struct Token
        {
            enum class Kind : uint8_t
            {
                Header_Name,
                /*
                    <stdio.h>
                    "foo.h"
                */

                Identifier,
                /*
                    same as Tokens::Token - starting with nondigit
                */

                PP_Number,
                /*
                    lax number - covers all possible integer and float forms
                    validation happens in PP-to-Token converter

                    examples: 1.2.3e+foo, 0x, 42uL, 3.14f
                */

                Character_Constant,
                /*
                    'c-char-sequence'
                    L'c-char-sequence'

                    raw - no escape resolution (Phase 5 not yet applied)
                */

                String_Literal,
                /*
                    "s-char-sequence"
                    L"s-char-sequence"

                    raw - no escape resolution
                */

                Operator,
                Punctuator,

                PP_Other,
                /*
                    any single non-whitespace character that doesn't fit above
                */

                Newline
            };

            enum class Specific : uint8_t
            {
                // Header names
                Header_Name_System,     // <stdio.h>
                Header_Name_Local,      // "foo.h"

                // Identifier
                Identifier,

                // PP_Number
                PP_Number,

                // Character constants
                Character_Constant,
                Character_Constant_Wide,    // L'...'

                // String literals
                String_Literal,
                String_Literal_Wide,        // L"..."

                // Operators - same as Tokens::Token
                Operator_Subscript_Open,    // [
                Operator_Subscript_Close,   // ]
                Operator_Paren_Open,        // (
                Operator_Paren_Close,       // )
                Operator_Dot,               // .
                Operator_Arrow,             // ->

                Operator_Increment,         // ++
                Operator_Decrement,         // --
                Operator_Ampersand,         // &
                Operator_Star,              // *
                Operator_Plus,              // +
                Operator_Minus,             // -
                Operator_BitwiseNot,        // ~
                Operator_LogicalNot,        // !

                Operator_Divide,            // /
                Operator_Modulo,            // %
                Operator_ShiftLeft,         // 
                Operator_ShiftRight,        // >>
                Operator_Less,              // 
                Operator_Greater,           // >
                Operator_LessEqual,         // <=
                Operator_GreaterEqual,      // >=
                Operator_Equal,             // ==
                Operator_NotEqual,          // !=
                Operator_BitwiseXor,        // ^
                Operator_BitwiseOr,         // |
                Operator_LogicalAnd,        // &&
                Operator_LogicalOr,         // ||

                Operator_Question,          // ?
                Operator_Colon,             // :

                Operator_Assign,            // =
                Operator_MulAssign,         // *=
                Operator_DivAssign,         // /=
                Operator_ModAssign,         // %=
                Operator_AddAssign,         // +=
                Operator_SubAssign,         // -=
                Operator_ShiftLeftAssign,   // <<=
                Operator_ShiftRightAssign,  // >>=
                Operator_AndAssign,         // &=
                Operator_XorAssign,         // ^=
                Operator_OrAssign,          // |=

                Operator_Comma,             // ,
                Operator_Hash,              // #
                Operator_HashHash,          // ##

                // Punctuators
                Punctuator_Brace_Open,      // {
                Punctuator_Brace_Close,     // }
                Punctuator_Semicolon,       // ;
                Punctuator_Ellipsis,        // ...

                PP_Other,

                Newline
            };

            Kind kind;

            Specific specific;
            StringPool::String value;

            uint64_t line;
            uint64_t column;
            StringPool::String file;

            uint64_t leadingWhitespaces;

            Token() {}

            Token(Kind k, Specific s, StringPool::String v, uint64_t l, uint64_t c, StringPool::String f, uint64_t lw)
                : kind(k), specific(s), value(v), line(l), column(c), file(f), leadingWhitespaces(lw) {}
        };

        void Print(std::ostream& out);

        inline const std::vector<Token>& GetTokens() const { return tokens; }
        inline std::vector<Token>& GetTokens() { return tokens; }
        inline void Push(const Token& token) { tokens.push_back(token); }
        inline void Reserve(std::vector<Token>::size_type n) { tokens.reserve(n); }
        inline void Clear() { tokens.clear(); }

        static std::optional<Token> GetOperator(const std::string& str, uint64_t lineNumber, uint64_t& pos, StringPool::String file, uint64_t leadingWhitespaces);

    private:
        std::vector<Token> tokens;

        friend class Tokenizer_Old;
    };

    class Tokens_Old
    {
    public:
        Tokens_Old() {}
        ~Tokens_Old() = default;

        struct Token
        {
            enum class Kind : uint8_t
            {
                Keyword,
                /*
                    auto     double int      struct
                    break    else   long     switch
                    case     enum   register typedef
                    char     extern return   union
                    const    float  short    unsigned
                    continue for    signed   void
                    default  goto   sizeof   volatile
                    do       if     static   while
                */

                Identifier,
                /*
                    nondigit: _, a-z, A-Z
                    digit: 0-9

                    starting with nondigit
                */

                Constant,
                /*
                    floating-constant
                        3.14
                        3.
                        .14
                        3.14e+10
                        3.e10

                        suffix: f/F, l/L

                    integer-constant
                        decimal-constant: starting nonzero-digit
                        octal-constant: starting with 0 and optional octal-digits after
                        hexadecimal-constant: starting with 0x/0X and hex-digits after

                        suffix:
                            unsigned: u/U
                            long: l/L

                    enumeration-constant
                        identifier

                    character-constant
                        'c-char-sequence'
                        L'c-char-sequence'

                        valid:
                            c-char                      ('a')
                            c-char-sequence c-char      ('abc')

                        simple-escape-sequence:
                            \' \" \? \\
                            \a \b \f \n \r \t \v

                        octal-escape-sequence:
                            \ octal-digit                           ('\7')
                            \ octal-digit octal-digit               ('\77')
                            \ octal-digit octal-digit octal-digit   ('\777')

                        hexadecimal-escape-sequence:
                            \x hexadecimal-digit                    ('\xF')
                            hexadecimal-escape-sequence hexadecimal-digit ('\xFF', '\x1A2B34CD')
                */

                String_Literal,
                /*
                    "s-char-sequence<optional>"
                    L"s-char-sequence<optional>"

                    escape-sequences the same
                */

                Operator,
                /*
                    [  ]  (  )  .  ->
                    ++ -- &  *  +  -  ~  !  sizeof
                    /  %  << >> <  >  <= >= == != ^  |  && ||
                    ?  :
                    =  *= /= %= += -= <<= >>= &= ^= |=
                    ,  #  ##
                */

                Punctuator
                /*
                    [  ]  (  )  {  }
                    *  ,  :  =  ;
                    ...  (Varargs)
                    #
                */

                /*
                    Overlap of Operator and Punctuator:

                    [  ]
                        Operator: a[i]
                        Punctuator: int a[] (array-declaration with size)

                    (  )
                        Operator: foo() (function call)
                        Punctuator: int foo() (parameter list)

                    *
                        Operator: Multiplication, Pointer-Dereference
                        Punctuator: Pointer-Declaration

                    ,
                        Operator: Comma-Operator in expression
                        Punctuator: foo(a, b) Argument-Separator

                    :
                        Operator: a ? b : c
                        Punctuator: case 5: (Label)

                    =
                        Operator: Assignment
                        Punctuator: Initializer
                */
            };

            enum class Specific : uint8_t
            {
                Keyword_Auto, Keyword_Register, Keyword_Static, Keyword_Extern,
                Keyword_Typedef,

                Keyword_Void, Keyword_Char, Keyword_Short, Keyword_Int,
                Keyword_Long, Keyword_Float, Keyword_Double, Keyword_Signed,
                Keyword_Unsigned,

                Keyword_Const, Keyword_Volatile,

                Keyword_Struct, Keyword_Union, Keyword_Enum,

                Keyword_If, Keyword_Else, Keyword_Switch, Keyword_Case, Keyword_Default,
                Keyword_For, Keyword_While, Keyword_Do, Keyword_Break, Keyword_Continue,

                Keyword_Return, Keyword_Goto,
                Keyword_Sizeof,


                Identifier,


                Constant_Float,

                Constant_Integer_Decimal,
                Constant_Integer_Octal,
                Constant_Integer_Hex,

                Constant_Character,
                Constant_Character_Wide,


                String_Literal,
                String_Literal_Wide,


                Operator_Subscript_Open,    // [
                Operator_Subscript_Close,   // ]
                Operator_Paren_Open,        // (
                Operator_Paren_Close,       // )
                Operator_Dot,               // .
                Operator_Arrow,             // ->

                Operator_Increment,         // ++
                Operator_Decrement,         // --
                Operator_Ampersand,         // &
                Operator_Star,              // *
                Operator_Plus,              // +
                Operator_Minus,             // -
                Operator_BitwiseNot,        // ~
                Operator_LogicalNot,        // !

                Operator_Divide,            // /
                Operator_Modulo,            // %
                Operator_ShiftLeft,         // <<
                Operator_ShiftRight,        // >>
                Operator_Less,              // <
                Operator_Greater,           // >
                Operator_LessEqual,         // <=
                Operator_GreaterEqual,      // >=
                Operator_Equal,             // ==
                Operator_NotEqual,          // !=
                Operator_BitwiseXor,        // ^
                Operator_BitwiseOr,         // |
                Operator_LogicalAnd,        // &&
                Operator_LogicalOr,         // ||

                Operator_Question,          // ?
                Operator_Colon,             // :

                Operator_Assign,            // =
                Operator_MulAssign,         // *=
                Operator_DivAssign,         // /=
                Operator_ModAssign,         // %=
                Operator_AddAssign,         // +=
                Operator_SubAssign,         // -=
                Operator_ShiftLeftAssign,   // <<=
                Operator_ShiftRightAssign,  // >>=
                Operator_AndAssign,         // &=
                Operator_XorAssign,         // ^=
                Operator_OrAssign,          // |=

                Operator_Comma,             // ,
                Operator_Hash,              // #
                Operator_HashHash,          // ##

                Punctuator_Brace_Open,     // {
                Punctuator_Brace_Close,    // }
                Punctuator_Semicolon,      // ;
                Punctuator_Ellipsis        // ...
            };

            Kind kind;

            Specific specific;
            StringPool::String value;

            uint64_t line;
            uint64_t column;
            StringPool::String file;

            Token() {}

            Token(Kind k, Specific s, StringPool::String v, uint64_t l, uint64_t c, StringPool::String f)
                : kind(k), specific(s), value(v), line(l), column(c), file(f) {}
        };

        void Print(std::ostream& out);

        inline const std::vector<Token>& GetTokens() const { return tokens; }

    private:
        std::vector<Token> tokens;

        friend class Tokenizer_Old;
    };

    class Tokenizer_Old
    {
    public:
        static PPTokens_Old Tokenize(Context& context, const std::vector<std::string>& filenames);

        static Tokens_Old Convert(Context& context, const PPTokens_Old& ppTokens);

        static void Serialize(Context& context, const PPTokens_Old& ppTokens, std::ostream& out);

    private:
        static void Tokenize(Context& context, const char* filename, PPTokens_Old& tokens, bool& warnedWithNonStandardSyntaxForLineDefinition);
    };

    class Tokenizer
    {
    public:
        

    private:
        
    };
}
