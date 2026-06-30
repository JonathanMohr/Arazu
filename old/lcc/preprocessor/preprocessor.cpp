#include "preprocessor.hpp"

#include <StringPool.hpp>
#include <context.hpp>
#include <info/warning.hpp>
#include <string>
#include <tokens.hpp>
#include <algorithm>
#include <stack>
#include <tuple>
#include <unordered_set>
#include <vector>
#include <filesystem>

#include <ctime>
#include <cstring>

std::tuple<C::PPTokens_Old, C::Dependencies> C::Preprocessor::Preprocess(Context& context, const PPTokens_Old& ppTokens_Old, const std::vector<std::string>& searchPaths)
{
    RobinHoodMap<StringPool::String, Macro> macros;
    std::unordered_set<StringPool::String> invalidMacros;

    StringPool::String predefinedFile = stringPool.GetString("<built-in>");

    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);

    char date[12]; // "Mmm dd yyyy"
    char time[9];  // "hh:mm:ss"

    if (tm == nullptr ||
        std::strftime(date, sizeof(date), "%b", tm) == 0 ||
        std::snprintf(date + 3, sizeof(date) - 3, " %2d %4d", tm->tm_mday, 1900 + tm->tm_year) < 0 ||
        std::strftime(time, sizeof(time), "%H:%M:%S", tm) == 0)
    { // TODO
        context.warningManager.add(LCC::Warning(LCC::Warning::Type::CouldNotGetDateAndTime, 0, 0, stringPool.GetString("")));
        std::strncpy(date, "Jan  1 1970", sizeof(date));
        std::strncpy(time, "00:00:00", sizeof(time));
    }

    auto createPredefinedMacro = [&](StringPool::String name, PPTokens_Old::Token::Kind kind, PPTokens_Old::Token::Specific specific, StringPool::String value)
    {
        Macro macro(std::vector{PPTokens_Old::Token(kind, specific, value, 0, 0, predefinedFile, false)}, false);
        invalidMacros.insert(name);
        macros[name] = std::move(macro);
    };

    // C89
    invalidMacros.insert(stringPool.GetString("defined"));
    // TODO: __LINE__, __FILE__
    createPredefinedMacro(stringPool.GetString("__LINE__"), PPTokens_Old::Token::Kind::PP_Number, PPTokens_Old::Token::Specific::PP_Number, stringPool.GetString("1")); // TODO: Set
    createPredefinedMacro(stringPool.GetString("__FILE__"), PPTokens_Old::Token::Kind::String_Literal, PPTokens_Old::Token::Specific::String_Literal, stringPool.GetString("file")); // TODO: Set
    createPredefinedMacro(stringPool.GetString("__DATE__"), PPTokens_Old::Token::Kind::String_Literal, PPTokens_Old::Token::Specific::String_Literal, stringPool.GetString(date));
    createPredefinedMacro(stringPool.GetString("__TIME__"), PPTokens_Old::Token::Kind::String_Literal, PPTokens_Old::Token::Specific::String_Literal, stringPool.GetString(time));
    createPredefinedMacro(stringPool.GetString("__STDC__"), PPTokens_Old::Token::Kind::PP_Number, PPTokens_Old::Token::Specific::PP_Number, stringPool.GetString("1"));

    TokenBufferPool tokenBufferPool;

    std::unordered_set<StringPool::String> includedFiles;

    Dependencies dependencies;

    C::PPTokens_Old tokens = Preprocess(
        context,
        tokenBufferPool,
        ppTokens_Old,
        macros,
        invalidMacros,
        includedFiles,
        searchPaths,
        dependencies,
        0
    );

    return std::make_tuple(std::move(tokens), std::move(dependencies));
}

namespace
{
    struct ConditionalState
    {
        StringPool::String file;
        uint64_t line;
        uint64_t column;

        bool inActiveBlock;
        bool active;
        bool anyBranchTrue;
        bool hadElse;
    };
}

C::PPTokens_Old C::Preprocessor::Preprocess(
    Context& context,
    TokenBufferPool& tokenBufferPool,
    const PPTokens_Old& ppTokens_Old,
    RobinHoodMap<StringPool::String, Macro>& macros,
    const std::unordered_set<StringPool::String>& invalidMacros,
    std::unordered_set<StringPool::String>& includedFiles,
    const std::vector<std::string>& searchPaths,
    Dependencies& dependencies,
    uint64_t depth
)
{
    PPTokens_Old result;
    result.Reserve(ppTokens_Old.GetTokens().size() * 2); // TODO: Estimate better

    const std::vector<PPTokens_Old::Token>& tokens = ppTokens_Old.GetTokens();
    std::vector<PPTokens_Old::Token>::size_type pos = 0;

    std::stack<ConditionalState> conditionalStack;

    StringPool::String file;
    if (tokens.size() > 0) file = tokens[0].file; // TODO
    uint64_t lineAddend = 0;

    while (pos < tokens.size())
    {
        Macro& lineMacro = *macros.find(stringPool.GetString("__LINE__"));
        Macro& fileMacro = *macros.find(stringPool.GetString("__FILE__"));

        lineMacro.replacement[0].value = stringPool.GetString(std::to_string(tokens[pos].line));
        fileMacro.replacement[0].value = tokens[pos].file;

        const bool currentConditionalActive = conditionalStack.empty() || (conditionalStack.top().active && conditionalStack.top().inActiveBlock);
        if (!currentConditionalActive)
        {
            const bool isConditionalPreprocessorDirective =
                tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Hash &&
                (pos == 0 || tokens[pos - 1].specific == PPTokens_Old::Token::Specific::Newline) &&
                pos + 1 < tokens.size() &&
                tokens[pos + 1].specific == PPTokens_Old::Token::Specific::Identifier &&
                (tokens[pos + 1].value == "if"     ||
                 tokens[pos + 1].value == "ifdef"  ||
                 tokens[pos + 1].value == "ifndef" ||
                 tokens[pos + 1].value == "elif"   ||
                 tokens[pos + 1].value == "else"   ||
                 tokens[pos + 1].value == "endif");
            
            if (!isConditionalPreprocessorDirective)
            {
                pos++;
                continue;
            }
        }

        if (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Hash &&
            (pos == 0 || tokens[pos - 1].specific == PPTokens_Old::Token::Specific::Newline))
        {
            pos++;

            if (pos >= tokens.size() || tokens[pos].specific == PPTokens_Old::Token::Specific::Newline) // #
            {
                if (pos < tokens.size()) pos++;
                continue;
            }

            // TODO: Fix tokens[pos] usage after if statement where pos >= tokens.size() could lead to that code being executed

            const PPTokens_Old::Token& directive = tokens[pos];
            pos++;

            if (directive.specific != PPTokens_Old::Token::Specific::Identifier)
                throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingDirective, directive.line, directive.column, directive.file);

            if (directive.value == "ifdef")
            {
                if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Identifier)
                    throw LCC::Exception(LCC::Exception::Type::MacroNameNotIdentifier, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                const StringPool::String name = tokens[pos].value;
                pos++;

                if (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    throw LCC::Exception(LCC::Exception::Type::TokenAfterPPDirective, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                const bool defined = macros.find(name) != nullptr;
                conditionalStack.push({ directive.file, directive.line, directive.column, currentConditionalActive, defined, defined, false });
            }
            else if (directive.value == "ifndef")
            {
                if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Identifier)
                    throw LCC::Exception(LCC::Exception::Type::MacroNameNotIdentifier, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                const StringPool::String name = tokens[pos].value;
                pos++;

                if (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    throw LCC::Exception(LCC::Exception::Type::TokenAfterPPDirective, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                const bool notDefined = macros.find(name) == nullptr;
                conditionalStack.push({ directive.file, directive.line, directive.column, currentConditionalActive, notDefined, notDefined, false });
            }
            else if (directive.value == "if")
            {
                const int64_t value = EvaluateConstantExpression(tokens, pos, macros, context, tokenBufferPool);
                const bool active = value != 0;

                conditionalStack.push({ directive.file, directive.line, directive.column, currentConditionalActive, active, active, false });
            }

            else if (directive.value == "elif")
            {
                if (conditionalStack.empty())
                    throw LCC::Exception(LCC::Exception::Type::ConditionalPPDirectiveWithoutIf, directive.line, directive.column, directive.file, "elif");

                ConditionalState& state = conditionalStack.top();

                if (state.hadElse)
                    throw LCC::Exception(LCC::Exception::Type::ConditionalPPElseAfterElse, directive.line, directive.column, directive.file);

                if (state.anyBranchTrue)
                {
                    state.active = false;
                }
                else
                {
                    const int64_t value = EvaluateConstantExpression(tokens, pos, macros, context, tokenBufferPool);
                    state.active = value != 0;
                    if (state.active)
                        state.anyBranchTrue = true;
                }
            }

            else if (directive.value == "else")
            {
                if (conditionalStack.empty())
                    throw LCC::Exception(LCC::Exception::Type::ConditionalPPDirectiveWithoutIf, directive.line, directive.column, directive.file, "else");

                if (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    throw LCC::Exception(LCC::Exception::Type::TokenAfterPPDirective, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                ConditionalState& state = conditionalStack.top();

                if (state.hadElse)
                    throw LCC::Exception(LCC::Exception::Type::ConditionalPPElseAfterElse, directive.line, directive.column, directive.file);

                state.active = !state.anyBranchTrue;
                state.hadElse = true;
            }

            else if (directive.value == "endif")
            {
                if (conditionalStack.empty())
                    throw LCC::Exception(LCC::Exception::Type::ConditionalPPDirectiveWithoutIf, directive.line, directive.column, directive.file, "endif");

                if (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    throw LCC::Exception(LCC::Exception::Type::TokenAfterPPDirective, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                conditionalStack.pop();
            }


            else if (directive.value == "include")
            {
                if (pos >= tokens.size() || tokens[pos].specific == PPTokens_Old::Token::Specific::Newline)
                    throw LCC::Exception(LCC::Exception::Type::NewlineAfterInclude, directive.line, directive.column, directive.file);

                bool isLocal = false;
                std::string filename;

                if (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Less)
                {
                    pos++; // skip <

                    while (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Greater && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    {
                        for (uint64_t i = 0; i < tokens[pos].leadingWhitespaces; i++)
                            filename += ' ';
                        filename += tokens[pos].value.str();
                        pos++;
                    }

                    if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Greater)
                        throw LCC::Exception(LCC::Exception::Type::UnterminatedIncludeDirective, directive.line, directive.column, directive.file);

                    pos++; // skip >
                }
                else
                {
                    std::vector<PPTokens_Old::Token>::size_type epos = 0;
                    PPTokens_Old expandedPP;
                    while (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                        ExpandToken(tokens, pos, expandedPP, macros, nullptr, context, tokenBufferPool, false);
                    

                    if (expandedPP.GetTokens().empty())
                        throw LCC::Exception(LCC::Exception::Type::NewlineAfterInclude, directive.line, directive.column, directive.file);

                    if (expandedPP.GetTokens()[epos].specific == PPTokens_Old::Token::Specific::Operator_Less)
                    {
                        epos++; // skip <
                        while (epos < expandedPP.GetTokens().size() && expandedPP.GetTokens()[epos].specific != PPTokens_Old::Token::Specific::Operator_Greater && expandedPP.GetTokens()[epos].specific != PPTokens_Old::Token::Specific::Newline)
                        {
                            for (uint64_t i = 0; i < expandedPP.GetTokens()[epos].leadingWhitespaces; i++)
                                filename += ' ';
                            filename += expandedPP.GetTokens()[epos].value.str();
                            epos++;
                        }

                        if (epos >= expandedPP.GetTokens().size() || expandedPP.GetTokens()[epos].specific != PPTokens_Old::Token::Specific::Operator_Greater)
                            throw LCC::Exception(LCC::Exception::Type::UnterminatedIncludeDirective, directive.line, directive.column, directive.file);

                        epos++; // skip >
                    }
                    else if (expandedPP.GetTokens()[epos].kind == PPTokens_Old::Token::Kind::String_Literal)
                    {
                        filename = expandedPP.GetTokens()[epos].value.str();
                        isLocal = true;
                        epos++;
                    }
                    else
                        throw LCC::Exception(LCC::Exception::Type::NewlineAfterInclude, directive.line, directive.column, directive.file);

                    if (epos < expandedPP.GetTokens().size() && expandedPP.GetTokens()[epos].specific != PPTokens_Old::Token::Specific::Newline)
                        throw LCC::Exception(LCC::Exception::Type::TokenAfterPPDirective, expandedPP.GetTokens()[epos].line, expandedPP.GetTokens()[epos].column, expandedPP.GetTokens()[epos].file);
                }

                if (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    throw LCC::Exception(LCC::Exception::Type::TokenAfterPPDirective, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                std::optional<std::string> resolvedPath = std::nullopt;
                if (isLocal)
                {
                    StringPool::String currentFile = directive.file;
                    std::filesystem::path localPath = std::filesystem::path(currentFile.str()).parent_path() / filename;
                    if (std::filesystem::exists(localPath))
                        resolvedPath = localPath.string();
                }

                if (!resolvedPath.has_value())
                {
                    for (const std::string& searchPath : searchPaths)
                    {
                        std::filesystem::path candidate = std::filesystem::path(searchPath) / filename;
                        if (std::filesystem::exists(candidate))
                        {
                            resolvedPath = candidate.string();
                            break;
                        }
                    }
                }

                if (!resolvedPath.has_value())
                    throw LCC::Exception(LCC::Exception::Type::IncludedFileNotFound, directive.line, directive.column, directive.file, filename);

                if (includedFiles.find(stringPool.GetString(resolvedPath.value())) != includedFiles.end())
                    throw LCC::Exception(LCC::Exception::Type::RecursiveInclude, directive.line, directive.column, directive.file);

                includedFiles.insert(stringPool.GetString(resolvedPath.value()));

                dependencies.Add(Dependencies::Dependency(stringPool.GetString(resolvedPath.value()), depth + 1));

                // TODO: Ugly
                std::vector<std::string> paths = { resolvedPath.value() };
                PPTokens_Old included = Tokenizer_Old::Tokenize(context, paths);
                PPTokens_Old preprocessed = Preprocess(context, tokenBufferPool, included, macros, invalidMacros, includedFiles, searchPaths, dependencies, depth + 1);

                for (const PPTokens_Old::Token& t : preprocessed.GetTokens())
                    result.Push(t);

                includedFiles.erase(stringPool.GetString(resolvedPath.value()));
            }


            else if (directive.value == "define")
            {
                // TODO: Warn when "defined" in replacement list

                if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Identifier)
                    throw LCC::Exception(LCC::Exception::Type::MacroNameNotIdentifier, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                const StringPool::String name = tokens[pos].value;
                if (invalidMacros.find(name) != invalidMacros.end())
                    throw LCC::Exception(LCC::Exception::Type::MacroInvalidName, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                pos++;

                Macro macro;

                if (pos < tokens.size() && 
                    tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Paren_Open &&
                    tokens[pos].leadingWhitespaces <= 0)
                {
                    macro.kind = Macro::Kind::Function;

                    pos++; // Skip (

                    if (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Paren_Close)
                    {
                        while (pos < tokens.size())
                        {
                            if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Identifier)
                                throw LCC::Exception(LCC::Exception::Type::MacroParameterNoIdentifier, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                            macro.parameters.push_back(tokens[pos].value);
                            pos++;

                            if (pos >= tokens.size())
                                throw LCC::Exception(LCC::Exception::Type::MacroUnterminatedParameterList, tokens[pos - 1].line, tokens[pos - 1].column, tokens[pos - 1].file);

                            if (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Paren_Close)
                                break;

                            if (tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Comma)
                                throw LCC::Exception(LCC::Exception::Type::MacroNoCommaInParameterList, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                            pos++; // Skip ,
                        }
                    }

                    if (pos >= tokens.size())
                        throw LCC::Exception(LCC::Exception::Type::MacroUnterminatedParameterList, tokens[pos - 1].line, tokens[pos - 1].column, tokens[pos - 1].file);

                    pos++; // Skip )
                }
                else
                {
                    macro.kind = Macro::Kind::Object;
                }

                while (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                {
                    if (tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_HashHash) macro.hasHashHash = true;
                    macro.replacement.push_back(tokens[pos]);
                    pos++;
                }

                ValidateReplacementList(macro);

                Macro* existing = macros.find(name);
                if (existing)
                {
                    // Redefinition — only when identical
                    if (existing->kind != macro.kind)
                        throw LCC::Exception(LCC::Exception::Type::MacroRedefinitionIncompatible,
                            directive.line, directive.column, directive.file);

                    if (macro.kind == Macro::Kind::Function)
                    {
                        if (existing->parameters.size() != macro.parameters.size())
                            throw LCC::Exception(LCC::Exception::Type::MacroRedefinitionIncompatible,
                                directive.line, directive.column, directive.file);

                        for (std::vector<std::string>::size_type i = 0; i < macro.parameters.size(); i++)
                        {
                            if (existing->parameters[i] != macro.parameters[i])
                                throw LCC::Exception(LCC::Exception::Type::MacroRedefinitionIncompatible,
                                    directive.line, directive.column, directive.file);
                        }
                    }

                    if (!ReplacementListsEqual(existing->replacement, macro.replacement))
                        throw LCC::Exception(LCC::Exception::Type::MacroRedefinitionIncompatible,
                            directive.line, directive.column, directive.file);
                }
                else
                {
                    macros[name] = std::move(macro);
                }
            }
            else if (directive.value == "undef")
            {
                if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Identifier)
                    throw LCC::Exception(LCC::Exception::Type::MacroNameNotIdentifier, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                const StringPool::String name = tokens[pos].value;
                if (invalidMacros.find(name) != invalidMacros.end())
                    throw LCC::Exception(LCC::Exception::Type::MacroInvalidName, tokens[pos].line, tokens[pos].column, tokens[pos].file);

                if (macros.find(name))
                    macros.erase(name);
                pos++;

                if (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    throw LCC::Exception(LCC::Exception::Type::TokenAfterPPDirective, tokens[pos].line, tokens[pos].column, tokens[pos].file);
            }


            else if (directive.value == "line")
            {
                std::vector<PPTokens_Old::Token>::size_type epos = 0;
                PPTokens_Old expandedPP;
                while (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    ExpandToken(tokens, pos, expandedPP, macros, nullptr, context, tokenBufferPool, false);

                if (epos >= expandedPP.GetTokens().size() || expandedPP.GetTokens()[epos].specific != PPTokens_Old::Token::Specific::PP_Number)
                    throw LCC::Exception(LCC::Exception::Type::LineNoPositiveInteger, expandedPP.GetTokens()[epos].line, expandedPP.GetTokens()[epos].column, expandedPP.GetTokens()[epos].file);

                const StringPool::String valueStr = expandedPP.GetTokens()[epos].value;
                epos++;

                // TODO: set line

                if (epos < expandedPP.GetTokens().size() && expandedPP.GetTokens()[epos].specific != PPTokens_Old::Token::Specific::Newline)
                {
                    if (epos >= expandedPP.GetTokens().size() || expandedPP.GetTokens()[epos].specific != PPTokens_Old::Token::Specific::String_Literal)
                        throw LCC::Exception(LCC::Exception::Type::LineInvalidFilename, expandedPP.GetTokens()[epos].line, expandedPP.GetTokens()[epos].column, expandedPP.GetTokens()[epos].file);

                    const StringPool::String newFilename = expandedPP.GetTokens()[epos].value;
                    epos++;

                    // TODO: set filename
                }

                if (epos < expandedPP.GetTokens().size() && expandedPP.GetTokens()[epos].specific != PPTokens_Old::Token::Specific::Newline)
                    throw LCC::Exception(LCC::Exception::Type::TokenAfterPPDirective, expandedPP.GetTokens()[epos].line, expandedPP.GetTokens()[epos].column, expandedPP.GetTokens()[epos].file);
            }

            else if (directive.value == "error")
            {
                std::string message;
                while (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                {
                    // TODO: Kind == Header_Name

                    if (!message.empty())
                    {
                        for (uint64_t i = 0; i < tokens[pos].leadingWhitespaces; i++)
                            message += ' ';
                    }

                    if (tokens[pos].specific == PPTokens_Old::Token::Specific::String_Literal)
                        message += '"';
                    if (tokens[pos].specific == PPTokens_Old::Token::Specific::String_Literal_Wide)
                        message += "L\"";
                    if (tokens[pos].specific == PPTokens_Old::Token::Specific::Character_Constant)
                        message += '\'';
                    if (tokens[pos].specific == PPTokens_Old::Token::Specific::Character_Constant_Wide)
                        message += "L'";

                    message += tokens[pos].value.str();
                    pos++;

                    if (tokens[pos].specific == PPTokens_Old::Token::Specific::String_Literal || tokens[pos].specific == PPTokens_Old::Token::Specific::String_Literal_Wide)
                        message += '"';
                    if (tokens[pos].specific == PPTokens_Old::Token::Specific::Character_Constant || tokens[pos].specific == PPTokens_Old::Token::Specific::Character_Constant_Wide)
                        message += '\'';
                }

                throw LCC::Exception(LCC::Exception::Type::UserDefinedError, directive.line, directive.column, directive.file, message);
            }


            else if (directive.value == "pragma")
            {
                // TODO

                while (pos < tokens.size() && tokens[pos].specific != PPTokens_Old::Token::Specific::Newline)
                    pos++;
            }


            else
                throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingDirective, directive.line, directive.column, directive.file);
            
        }
        else
        {
            ExpandToken(tokens, pos, result, macros, nullptr, context, tokenBufferPool, false);
        }
    }

    if (!conditionalStack.empty())
        throw LCC::Exception(LCC::Exception::Type::UnterminatedConditionalPPDirective, conditionalStack.top().line, conditionalStack.top().column, conditionalStack.top().file);

    return result;
}

void C::Preprocessor::ValidateReplacementList(const C::Preprocessor::Macro& macro)
{
    const std::vector<PPTokens_Old::Token>& replacement = macro.replacement;

    if (!replacement.empty())
    {
        if (replacement.front().specific == PPTokens_Old::Token::Specific::Operator_HashHash)
            throw LCC::Exception(LCC::Exception::Type::HashHashAtStartOfReplacementList, replacement.front().line, replacement.front().column, replacement.front().file);

        if (replacement.back().specific == PPTokens_Old::Token::Specific::Operator_HashHash)
            throw LCC::Exception(LCC::Exception::Type::HashHashAtEndOfReplacementList, replacement.front().line, replacement.front().column, replacement.front().file);
    }

    if (macro.kind == C::Preprocessor::Macro::Kind::Function)
    {
        for (std::vector<PPTokens_Old::Token>::size_type i = 0; i < replacement.size(); i++)
        {
            if (replacement[i].specific == PPTokens_Old::Token::Specific::Operator_Hash)
            {
                if (i + 1 >= replacement.size() ||
                    replacement[i + 1].specific != PPTokens_Old::Token::Specific::Identifier ||
                    std::find(macro.parameters.begin(), macro.parameters.end(),
                              replacement[i + 1].value) == macro.parameters.end())
                    throw LCC::Exception(LCC::Exception::Type::HashOperatorNotFollowedByParameter,
                        replacement[i].line, replacement[i].column, replacement[i].file);
            }
        }
    }
}

bool C::Preprocessor::ReplacementListsEqual(
    const std::vector<PPTokens_Old::Token>& a,
    const std::vector<PPTokens_Old::Token>& b
)
{
    if (a.size() != b.size())
        return false;

    for (std::vector<PPTokens_Old::Token>::size_type i = 0; i < a.size(); i++)
    {
        if (a[i].specific != b[i].specific)
            return false;
        if (a[i].value != b[i].value)
            return false;

        // Per C89: all white-space separations are considered identical,
        // but presence vs absence of whitespace matters
        if (i > 0 &&( a[i].leadingWhitespaces > 0) != (b[i].leadingWhitespaces > 0))
            return false;
    }

    return true;
}

void C::Preprocessor::ApplyHashHash(const std::vector<PPTokens_Old::Token>& tokens, std::vector<PPTokens_Old::Token>& result)
{
    result.clear();
    result.reserve(tokens.size());

    for (std::vector<PPTokens_Old::Token>::size_type i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].specific == PPTokens_Old::Token::Specific::Operator_HashHash)
        {
            if (result.empty())
                throw LCC::Exception(LCC::Exception::Type::HashHashAtStartOfReplacementList,
                    tokens[i].line, tokens[i].column, tokens[i].file);

            if (i + 1 >= tokens.size())
                throw LCC::Exception(LCC::Exception::Type::HashHashAtEndOfReplacementList,
                    tokens[i].line, tokens[i].column, tokens[i].file);

            PPTokens_Old::Token concatenated = result.back();
            result.pop_back();

            const PPTokens_Old::Token& next = tokens[i + 1];

            if (concatenated.kind == PPTokens_Old::Token::Kind::String_Literal ||
                concatenated.kind == PPTokens_Old::Token::Kind::Character_Constant ||
                next.kind == PPTokens_Old::Token::Kind::String_Literal ||
                next.kind == PPTokens_Old::Token::Kind::Character_Constant)
                throw LCC::Exception(LCC::Exception::Type::HashHashInvalidConcatenation, tokens[i].line, tokens[i].column, tokens[i].file);

            if (
                (concatenated.kind == PPTokens_Old::Token::Kind::Identifier &&
                (next.kind == PPTokens_Old::Token::Kind::Identifier ||
                next.kind == PPTokens_Old::Token::Kind::PP_Number)) ||

                (concatenated.kind == PPTokens_Old::Token::Kind::PP_Number &&
                (next.kind == PPTokens_Old::Token::Kind::PP_Number ||
                next.kind == PPTokens_Old::Token::Kind::Identifier ||
                (next.kind == PPTokens_Old::Token::Kind::Operator && next.value == '.')))
            )
            {
                concatenated.value = concatenated.value + next.value;
            }

            else if (concatenated.kind == PPTokens_Old::Token::Kind::Operator && concatenated.value == '.' &&
                     next.kind == PPTokens_Old::Token::Kind::PP_Number)
            {
                concatenated.value = concatenated.value + next.value;
                concatenated.kind = PPTokens_Old::Token::Kind::PP_Number;
                concatenated.specific = PPTokens_Old::Token::Specific::PP_Number;
            }
            else if ((concatenated.kind == PPTokens_Old::Token::Kind::Operator ||
                      concatenated.kind == PPTokens_Old::Token::Kind::Punctuator) &&
                     (next.kind == PPTokens_Old::Token::Kind::Operator ||
                      next.kind == PPTokens_Old::Token::Kind::Punctuator))
            {
                const StringPool::String combined = concatenated.value + next.value;

                uint64_t tempPos = 0;
                std::optional<PPTokens_Old::Token> newOperator = PPTokens_Old::GetOperator(combined.str(), concatenated.line, tempPos, concatenated.file, concatenated.leadingWhitespaces);

                if (!newOperator.has_value())
                    throw LCC::Exception(LCC::Exception::Type::HashHashInvalidConcatenation, tokens[i].line, tokens[i].column, tokens[i].file);

                PPTokens_Old::Token op = newOperator.value();
                op.column = concatenated.column;
                concatenated = op;
            }

            else
                throw LCC::Exception(LCC::Exception::Type::HashHashInvalidConcatenation, tokens[i].line, tokens[i].column, tokens[i].file);

            result.push_back(concatenated);
            i++;
        }
        else
        {
            result.push_back(tokens[i]);
        }
    }
}

std::string C::Preprocessor::StringifyArgument(const std::vector<PPTokens_Old::Token>& tokens)
{
    std::string result;

    for (std::vector<PPTokens_Old::Token>::size_type i = 0; i < tokens.size(); i++)
    {
        for (uint64_t j = 0; j < tokens[i].leadingWhitespaces; j++)
            result += ' ';

        if (tokens[i].kind == C::PPTokens_Old::Token::Kind::String_Literal ||
            tokens[i].kind == C::PPTokens_Old::Token::Kind::Character_Constant)
        {
            const bool isWide =
                tokens[i].specific == C::PPTokens_Old::Token::Specific::String_Literal_Wide ||
                tokens[i].specific == C::PPTokens_Old::Token::Specific::Character_Constant_Wide;
            const bool isString = tokens[i].kind == C::PPTokens_Old::Token::Kind::String_Literal;
            const char delim = isString ? '"' : '\'';

            if (isWide)
                result += 'L';

            result += '\\';
            result += delim;

            // Escape every \ and " in the raw value
            for (char c : tokens[i].value.str())
            {
                if (c == '\\' || c == '"')
                    result += '\\';
                result += c;
            }

            result += '\\';
            result += delim;
        }
        else
        {
            result += tokens[i].value.str();
        }
    }

    return result;
}

void C::Preprocessor::Substitute(
    const std::vector<PPTokens_Old::Token>& replacement,
    const std::vector<StringPool::String>& params,
    const std::vector<std::vector<PPTokens_Old::Token>>& args,
    const std::vector<std::vector<PPTokens_Old::Token>>& expandedArgs,
    std::vector<PPTokens_Old::Token>& result
)
{
    result.reserve(replacement.size() * 2); // TODO: Better estimate

    for (std::vector<PPTokens_Old::Token>::size_type i = 0; i < replacement.size(); i++)
    {
        const PPTokens_Old::Token& token = replacement[i];

        if (token.specific == PPTokens_Old::Token::Specific::Operator_Hash)
        {
            i++;

            const StringPool::String paramName = replacement[i].value;

            // TODO: Check if valid, should already be checked at define time
            auto paramIt = std::find(params.begin(), params.end(), paramName);
            const std::vector<C::PPTokens_Old::Token>& arg = args[static_cast<std::vector<C::PPTokens_Old::Token>::size_type>(paramIt - params.begin())];

            PPTokens_Old::Token strToken = replacement[i];
            strToken.kind = PPTokens_Old::Token::Kind::String_Literal;
            strToken.specific = PPTokens_Old::Token::Specific::String_Literal;
            strToken.value = stringPool.GetString(StringifyArgument(arg));
            strToken.leadingWhitespaces = token.leadingWhitespaces;
            result.push_back(strToken);
        }
        else if (token.specific == PPTokens_Old::Token::Specific::Identifier)
        {
            auto paramIt = std::find(params.begin(), params.end(), token.value);

            if (paramIt != params.end())
            {
                const std::vector<C::PPTokens_Old::Token>::size_type pIdx = static_cast<std::vector<C::PPTokens_Old::Token>::size_type>(paramIt - params.begin());

                // TODO: Check if correct
                const bool prevIsHashHash = i > 0 &&
                    replacement[i - 1].specific == PPTokens_Old::Token::Specific::Operator_HashHash;
                const bool nextIsHashHash = i + 1 < replacement.size() &&
                    replacement[i + 1].specific == PPTokens_Old::Token::Specific::Operator_HashHash;

                const std::vector<C::PPTokens_Old::Token>& src =
                    (prevIsHashHash || nextIsHashHash) ? args[pIdx] : expandedArgs[pIdx];

                for (const C::PPTokens_Old::Token& t : src)
                {
                    result.push_back(t);
                }
            }
            else
            {
                result.push_back(token);
            }
        }
        else
        {
            result.push_back(token);
        }
    }
}

void C::Preprocessor::ExpandToken(
    const std::vector<PPTokens_Old::Token>& tokens,
    std::vector<PPTokens_Old::Token>::size_type& pos,
    PPTokens_Old& result,
    RobinHoodMap<StringPool::String, Macro>& macros,
    const HideSetFrame* parentHideSet,
    Context& context,
    TokenBufferPool& tokenBufferPool,
    bool conditionalDirective
)
{
    auto isHidden = [](StringPool::String name, const HideSetFrame* frame) -> bool
    {
        while (frame)
        {
            if (frame->name == name) return true;
            frame = frame->parent;
        }
        return false;
    };

    const PPTokens_Old::Token& token = tokens[pos];

    // Only identifiers can be macro names
    if (token.specific != PPTokens_Old::Token::Specific::Identifier)
    {
        result.Push(token);
        pos++;
        return;
    }

    if (conditionalDirective && tokens[pos].value == "defined")
    {
        result.Push(tokens[pos]);
        pos++;

        if (pos < tokens.size() && tokens[pos].specific == PPTokens_Old::Token::Specific::Operator_Paren_Open)
        {
            result.Push(tokens[pos]);
            pos++; // skip (

            if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Identifier)
                throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression, // TODO: Could be more specific
                    token.line, token.column, token.file);

            result.Push(tokens[pos]);
            pos++;

            if (pos >= tokens.size() || tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Paren_Close)
                throw LCC::Exception(LCC::Exception::Type::InvalidPreprocessingExpression, // TODO: Could be more specific
                    token.line, token.column, token.file);

            result.Push(tokens[pos]);
            pos++; // skip )
        }
        else if (pos < tokens.size())
        {
            result.Push(tokens[pos]);
            pos++;
        }

        return;
    }

    const StringPool::String name = token.value;
    Macro* it = macros.find(name);

    // Not a macro, or in hide set (currently being expanded — no recursive replacement)
    if (!it || isHidden(name, parentHideSet))
    {
        result.Push(token);
        pos++;
        return;
    }

    C::Preprocessor::Macro& macro = *it;

    if (macro.kind == C::Preprocessor::Macro::Kind::Object)
    {
        pos++;

        HideSetFrame hideSetFrame{ token.value, parentHideSet };

        TokenBufferPool::Buffer afterHashHash = tokenBufferPool.Acquire();
        if (macro.hasHashHash)
            ApplyHashHash(macro.replacement, afterHashHash.get());
    
        const std::vector<PPTokens_Old::Token>& source = macro.hasHashHash ? afterHashHash.get() : macro.replacement;

        std::vector<PPTokens_Old::Token>::size_type rpos = 0;
        bool first = true;

        while (rpos < source.size())
        {
            uint64_t insertIndex = result.GetTokens().size();

            ExpandToken(source, rpos, result, macros, &hideSetFrame, context, tokenBufferPool, conditionalDirective);

            // TODO
            // for (uint64_t i = insertIndex; i < result.GetTokens().size(); i++)
            // {
            //     PPTokens_Old::Token& newToken = result.GetTokens()[i];
            //     newToken.line = token.line;
            //     newToken.file = token.file;
            // }

            if (first && result.GetTokens().size() > insertIndex)
            {
                PPTokens_Old::Token& firstNewToken = result.GetTokens()[insertIndex];
                // TODO: firstNewToken.line = token.line;
                firstNewToken.leadingWhitespaces = token.leadingWhitespaces;
                
                first = false;
            }
        }
    }
    else // Function-like
    {
        pos++; // skip macro name

        // Skip newlines — inside macro invocation, newline is whitespace
        while (pos < tokens.size() &&
               tokens[pos].specific == PPTokens_Old::Token::Specific::Newline)
            pos++;

        // If not followed by (, this is not a macro invocation — treat as identifier
        if (pos >= tokens.size() ||
            tokens[pos].specific != PPTokens_Old::Token::Specific::Operator_Paren_Open)
        {
            result.Push(token);
            return;
        }

        pos++; // skip (

        std::vector<std::vector<PPTokens_Old::Token>> args;
        args.reserve(macro.parameters.size());
        std::vector<PPTokens_Old::Token> currentArg;

        uint64_t depth = 1;

        while (pos < tokens.size() && depth > 0)
        {
            const PPTokens_Old::Token& t = tokens[pos];

            if (t.specific == PPTokens_Old::Token::Specific::Operator_Paren_Open)
            {
                depth++;
                currentArg.push_back(t);
            }
            else if (t.specific == PPTokens_Old::Token::Specific::Operator_Paren_Close)
            {
                depth--;
                if (depth > 0)
                    currentArg.push_back(t);
            }
            else if (t.specific == PPTokens_Old::Token::Specific::Operator_Comma && depth == 1)
            {
                args.push_back(std::move(currentArg));
                currentArg.clear();
            }
            else if (t.specific == PPTokens_Old::Token::Specific::Newline)
            {
                // Newline is whitespace inside macro invocation — skip
            }
            else
            {
                currentArg.push_back(t);
            }

            pos++;
        }

        if (depth != 0)
            throw LCC::Exception(LCC::Exception::Type::MacroUnterminatedInvocation,
                token.line, token.column, token.file);

        // For zero-parameter macros called as FOO(): args should be empty
        // For n-parameter macros: push the last collected argument
        if (!macro.parameters.empty() || !currentArg.empty())
            args.push_back(std::move(currentArg));

        // Validate argument count
        if (args.size() != macro.parameters.size())
            throw LCC::Exception(LCC::Exception::Type::MacroWrongArgumentCount,
                token.line, token.column, token.file);

        // Per C89: if any argument is empty, behavior is undefined — treat as error
        for (std::vector<std::vector<PPTokens_Old::Token>>::size_type i = 0; i < args.size(); i++)
        {
            if (args[i].empty())
                throw LCC::Exception(LCC::Exception::Type::MacroEmptyArgument,
                    token.line, token.column, token.file);
        }

        // Argument-Expansion
        macro.needsExpansion.assign(macro.parameters.size(), false);
        for (std::vector<PPTokens_Old::Token>::size_type i = 0; i < macro.replacement.size(); i++)
        {
            const PPTokens_Old::Token& rt = macro.replacement[i];
            if (rt.specific != PPTokens_Old::Token::Specific::Identifier)
                continue;

            // Skip operands of '#' (they are stringified, not expanded)
            if (i > 0 && macro.replacement[i - 1].specific == PPTokens_Old::Token::Specific::Operator_Hash)
                continue;

            auto pIt = std::find(macro.parameters.begin(), macro.parameters.end(), rt.value);
            if (pIt == macro.parameters.end())
                continue;

            const bool prevHH = i > 0 &&
                macro.replacement[i - 1].specific == PPTokens_Old::Token::Specific::Operator_HashHash;
            const bool nextHH = i + 1 < macro.replacement.size() &&
                macro.replacement[i + 1].specific == PPTokens_Old::Token::Specific::Operator_HashHash;

            if (!prevHH && !nextHH)
                macro.needsExpansion[static_cast<std::vector<bool>::size_type>(pIt - macro.parameters.begin())] = true;
        }

        std::vector<std::vector<PPTokens_Old::Token>> expandedArgs(args.size());
        for (std::vector<std::vector<PPTokens_Old::Token>>::size_type i = 0; i < args.size(); i++)
        {
            if (!macro.needsExpansion[i])
                continue;

            if (args.size() > 0) // TODO
            {
                Macro& lineMacro = *macros.find(stringPool.GetString("__LINE__"));
                Macro& fileMacro = *macros.find(stringPool.GetString("__FILE__"));

                lineMacro.replacement[0].value = stringPool.GetString(std::to_string(args[i][0].line));
                fileMacro.replacement[0].value = args[i][0].file;
            }

            PPTokens_Old expanded;
            std::vector<PPTokens_Old::Token>::size_type argPos = 0;
            while (argPos < args[i].size())
                ExpandToken(args[i], argPos, expanded, macros, parentHideSet, context, tokenBufferPool, conditionalDirective);
            expandedArgs[i] = std::move(expanded.GetTokens());
        }

        HideSetFrame hideSetFrame{ token.value, parentHideSet };

        TokenBufferPool::Buffer substituted = tokenBufferPool.Acquire();
        Substitute(macro.replacement, macro.parameters, args, expandedArgs, substituted.get());

        TokenBufferPool::Buffer afterHashHash = tokenBufferPool.Acquire();
        if (macro.hasHashHash)
            ApplyHashHash(substituted.get(), afterHashHash.get());
    
        const std::vector<PPTokens_Old::Token>& source = macro.hasHashHash ? afterHashHash.get() : substituted.get();

        std::vector<PPTokens_Old::Token>::size_type rpos = 0;
        bool first = true;

        while (rpos < source.size())
        {
            uint64_t insertIndex = result.GetTokens().size();

            ExpandToken(source, rpos, result, macros, &hideSetFrame, context, tokenBufferPool, conditionalDirective);

            // TODO
            // for (uint64_t i = insertIndex; i < result.GetTokens().size(); i++)
            // {
            //     PPTokens_Old::Token& newToken = result.GetTokens()[i];
            //     newToken.line = token.line;
            //     newToken.file = token.file;
            // }

            if (first && result.GetTokens().size() > insertIndex)
            {
                PPTokens_Old::Token& firstNewToken = result.GetTokens()[insertIndex];
                // TODO: firstNewToken.line = token.line;
                firstNewToken.leadingWhitespaces = token.leadingWhitespaces;
                
                first = false;
            }
        }
    }
}
