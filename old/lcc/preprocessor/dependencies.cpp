#include "preprocessor.hpp"

#include <cstdint>
#include <unordered_set>

static uint64_t PutBackspacesOut(StringPool::String file, std::ostream& out, bool string)
{
    if (file.str().find('\\') == std::string::npos &&
        (file.str().find(' ') == std::string::npos || string) &&
        file.str().find('"') == std::string::npos)
    {
        out << file.str();
        return file.size();
    }

    uint64_t n = 0;
    for (uint64_t i = 0; i < file.size(); i++)
    {
        const char c = file[i];
        
        if (c == '\\')
        {
            out << "\\\\";
            n += 2;
        }
        else if (c == ' ' && !string)
        {
            out << "\\ ";
            n += 2;
        }
        else if (c == '"')
        {
            out << "\\\"";
            n += 2;
        }
        else
        {
            out << c;
            n++;
        }
    }

    return n;
}

void C::Dependencies::Dump(Format format, std::vector<std::string>& sources, std::ostream& out, const std::string& target)
{
    std::unordered_set<StringPool::String> alreadyPutThere;

    const uint64_t maxLineLengthMake = 90;
    uint64_t currentLineLengthMake = 0;

    bool firstJson = true;

    auto DumpDependency = [format, &out, &alreadyPutThere, &currentLineLengthMake, &firstJson](StringPool::String dependency, uint64_t depth) {
        if (format != Format::MSVC)
        {
            if (alreadyPutThere.find(dependency) != alreadyPutThere.end()) return;
            alreadyPutThere.insert(dependency);
        }

        if (format == Format::MSVC)
        {
            out << "Note: including file:";
            for (uint64_t i = 0; i < depth; i++)
                out << ' ';
        }
        else if (format == Format::Makefile || format == Format::MakefileDummies)
        {
            if (currentLineLengthMake + dependency.size() + 1 > maxLineLengthMake)
            {
                out << " \\\n  ";
                currentLineLengthMake = 2;
            }
            else
            {
                out << ' ';
                currentLineLengthMake++;
            }

            currentLineLengthMake += PutBackspacesOut(dependency, out, false);
        }
        else if (format == Format::JSON)
        {
            if (!firstJson)
            {
                out << ",\n";
            }
            else
                firstJson = false;

            out << "        \"";
            PutBackspacesOut(dependency, out, true);
            out << '"';
        }

        if (format != Format::Makefile && format != Format::MakefileDummies && format != Format::JSON)
            out << dependency << '\n';
    };

    if (format == Format::Makefile || format == Format::MakefileDummies)
    {
        currentLineLengthMake += PutBackspacesOut(stringPool.GetString(target), out, false) + 1;
        out << ':';
    }
    else if (format == Format::JSON)
    {
        out << "{\n" << "    \"";
        PutBackspacesOut(stringPool.GetString(target), out, true);
        out << "\": [\n";
    }

    if (format != Format::MSVC)
    {
        for (const std::string& source : sources)
        {
            DumpDependency(stringPool.GetString(source), 0);
        }
    }

    for (const Dependency& dependency : dependencies)
    {
        DumpDependency(dependency.path, dependency.depth);
    }

    if (format == Format::Makefile || format == Format::MakefileDummies)
        out << '\n';

    if (format == Format::MakefileDummies)
    {
        // TODO: Maybe source not
        for (const std::string& source : sources)
        {
            PutBackspacesOut(stringPool.GetString(source), out, false);
            out << ":\n";
        }

        for (const Dependency& dependency : dependencies)
        {
            PutBackspacesOut(dependency.path, out, false);
            out << ":\n";
        }
    }
    else if (format == Format::JSON)
    {
        out << "\n    ]\n}\n";
    }
}
