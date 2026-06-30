#pragma once

#include <StringPool.hpp>
#include <tokens.hpp>
#include <unordered_set>
#include <vector>
#include <cstdint>
#include <hashmap.hpp>

namespace C
{
    class Dependencies
    {
    public:
        struct Dependency
        {
            inline Dependency(StringPool::String p, uint64_t d) : path(p), depth(d) {}

            StringPool::String path;
            uint64_t depth;
        };

        enum class Format : uint8_t
        {
            Normal,
            Makefile,
            MakefileDummies,
            MSVC,
            JSON
        };

        inline void Add(Dependency dep) { dependencies.push_back(dep); }

        void Dump(Format format, std::vector<std::string>& sources, std::ostream& out, const std::string& target);

    private:
        std::vector<Dependency> dependencies;
    };

    class Preprocessor
    {
    public:
        static std::tuple<PPTokens_Old, Dependencies> Preprocess(Context& context, const PPTokens_Old& ppTokens_Old, const std::vector<std::string>& searchPaths);

    private:
        class TokenBufferPool
        {
        public:
            class Buffer
            {
            public:
                inline std::vector<PPTokens_Old::Token>& get() { return buffer; }
                inline const std::vector<PPTokens_Old::Token>& get() const { return buffer; }

                inline ~Buffer() { if (pool) pool->Release(std::move(buffer)); }
                
                inline Buffer(Buffer&& other) noexcept : pool(other.pool), buffer(std::move(other.buffer)) { other.pool = nullptr; }
                Buffer& operator=(Buffer&&) = delete;
                Buffer(const Buffer&) = delete;
                Buffer& operator=(const Buffer&) = delete;

            private:
                friend class TokenBufferPool;
        
                inline Buffer(TokenBufferPool& p, std::vector<PPTokens_Old::Token>&& b) : pool(&p), buffer(std::move(b)) {}
                
                TokenBufferPool* pool;
                std::vector<PPTokens_Old::Token> buffer;
            };

            inline TokenBufferPool() { available.reserve(64); }
            inline ~TokenBufferPool() = default;

            Buffer Acquire();

        private:
            void Release(std::vector<PPTokens_Old::Token>&& buf);

            std::vector<std::vector<PPTokens_Old::Token>> available;
        };

        struct HideSetFrame
        {
            StringPool::String name;
            const HideSetFrame* parent;
        };

        struct Macro
        {
            enum class Kind : uint8_t
            {
                Object,
                Function
            };

            Kind kind;
            std::vector<StringPool::String> parameters;
            std::vector<PPTokens_Old::Token> replacement;

            bool hasHashHash = false;
            std::vector<bool> needsExpansion;

            Macro() {}

            Macro(std::vector<PPTokens_Old::Token> _replacement, bool hashHash)
                : kind(Kind::Object), replacement(std::move(_replacement)), hasHashHash(hashHash) {};

            Macro(std::vector<StringPool::String> _parameters, std::vector<PPTokens_Old::Token> _replacement, bool hashHash)
                : kind(Kind::Function), parameters(std::move(_parameters)), replacement(std::move(_replacement)), hasHashHash(hashHash) {};
        };

        static PPTokens_Old Preprocess(
            Context& context,
            TokenBufferPool& tokenBufferPool,
            const PPTokens_Old& ppTokens_Old,
            RobinHoodMap<StringPool::String, Macro>& macros,
            const std::unordered_set<StringPool::String>& invalidMacros,
            std::unordered_set<StringPool::String>& includedFiles,
            const std::vector<std::string>& searchPaths,
            Dependencies& dependencies,
            uint64_t depth
        );

        static int64_t EvaluateConstantExpression(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context,
            TokenBufferPool& tokenBufferPool
        );

        static int64_t ParseConditional(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseLogicalOr(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseLogicalAnd(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseBinaryOr(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseBinaryXor(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseBinaryAnd(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseEquality(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseRelational(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseShift(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseAddSub(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseMulDiv(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParseUnary(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );
        static int64_t ParsePrimary(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            const RobinHoodMap<StringPool::String, Macro>& macros,
            Context& context
        );

        static void ValidateReplacementList(const Macro& macro);
        static bool ReplacementListsEqual(const std::vector<PPTokens_Old::Token>& a, const std::vector<PPTokens_Old::Token>& b);

        static void ApplyHashHash(const std::vector<PPTokens_Old::Token>& tokens, std::vector<PPTokens_Old::Token>& result);
        static std::string StringifyArgument(const std::vector<PPTokens_Old::Token>& tokens);

        static void Substitute(
            const std::vector<PPTokens_Old::Token>& replacement,
            const std::vector<StringPool::String>& params,
            const std::vector<std::vector<PPTokens_Old::Token>>& args,
            const std::vector<std::vector<PPTokens_Old::Token>>& expandedArgs,
            std::vector<PPTokens_Old::Token>& result
        );

        static void ExpandToken(
            const std::vector<PPTokens_Old::Token>& tokens,
            std::vector<PPTokens_Old::Token>::size_type& pos,
            PPTokens_Old& result,
            RobinHoodMap<StringPool::String, Macro>& macros,
            const HideSetFrame* parentHideSet,
            Context& context,
            TokenBufferPool& tokenBufferPool,
            bool conditionalDirective
        );
    };
}
