#pragma once

#include <algorithm>
#include <array>
#include <string_view>

#include <util/string.hpp>
#include "../parser.hpp"

namespace Assembler
{
    namespace x86
    {
        static constexpr std::array<std::string_view, 16> dataDefinitions = {
            "db", "dw", "dd", "dq", "dt", "do", "dy", "dz",
            "resb", "resw", "resd", "resq", "rest", "reso", "resy", "resz"
        };

        static inline bool isDataDefinition(const Tokens::Token& token)
        {
            return token.type == Tokens::Type::Token && std::find(dataDefinitions.begin(), dataDefinitions.end(), toLower(token.value)) != dataDefinitions.end();
        }
    }
}
