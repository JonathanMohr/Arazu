#pragma once

#include <string>
#include <Architecture.hpp>
#include "../context.hpp"

bool parseArguments(int argc, const char *argv[],
                    std::vector<std::string>& inputs, std::string& output,
                    BitMode& bits, Architecture& arch, Format& format,
                    bool& debug, Context& context);
