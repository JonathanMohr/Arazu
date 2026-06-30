#pragma once

#include <Exception.hpp>
#include <StringPool.hpp>

struct Context
{
    WarningManager* warningManager;
    StringPool* stringPool;
    StringPool::String filename;
};
