#pragma once

#include <cstdint>
#include <StringPool.hpp>

uint64_t evalInteger(StringPool::String str, size_t size, uint64_t lineNumber, uint64_t column, const char* filename = nullptr);
