#pragma once

#include <StringPool.hpp>
#include "info/info.hpp" // IWYU pragma: keep

extern StringPool stringPool;

struct Context
{
    LCC::WarningManager warningManager;
};
