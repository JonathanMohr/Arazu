#include <iostream>
#include <string>

#include <io/file.hpp>
#include <Architecture.hpp>
#include <Exception.hpp>
#include <StringPool.hpp>
#include "asmprog/asmprog.hpp"
#include "cli/Arguments.hpp"
#include "context.hpp"

#include "parser/parser.hpp"
#include "encoder/encoder.hpp"
#include "objectwriter/objectwriter.hpp"

static int handleError(const std::exception& e)
{
    std::cerr << e.what() << '\n';
    return 1;
}

int main(int argc, const char *argv[])
{
    // TODO: remove this once it's finished
    std::cerr << "Warning: Assembler isn't stable yet and could have bugs" << '\n';

    WarningManager warningManager;
    Context context;
    context.warningManager = &warningManager;

    StringPool stringPool;
    context.stringPool = &stringPool;

    OutputStream objectFile;
    try
    {
        std::vector<std::string> inputFiles;
        std::string outputFile;
        BitMode bitMode;
        Architecture arch;
        Format format;
        bool debug;

        bool stop = parseArguments(argc, argv, inputFiles, outputFile, bitMode, arch, format, debug, context);
        if (stop) return 0;

        context.filename = stringPool.GetString(inputFiles.at(0));

        Assembler::Tokens tokens = Assembler::Tokenizer::Tokenize(context, inputFiles);
        if (debug) tokens.Print(std::cerr);

        std::unique_ptr<Assembler::Parser> parser = Assembler::Parser::Get(context, arch, bitMode);
        if (!parser) throw Exception::InternalError("Could not get parser", 0, 0);

        std::unique_ptr<Assembler::ObjectWriter> objectWriter = Assembler::ObjectWriter::Get(context, format);
        if (!objectWriter) throw Exception::InternalError("Could not get object writer", 0, 0);

        Assembler::AsmProg asmProg = parser->Parse(tokens);
        Assembler::Object object = Assembler::Encoder::Encode(asmProg, context);

        objectFile = openOutputStream(outputFile, std::ios::out | std::ios::trunc | std::ios::binary);
        objectWriter->Write(object, objectFile);

        if (warningManager.hasWarnings())
        {
            warningManager.printAll(std::cerr);
            warningManager.clear();
        }
    }
    catch(const Exception& e)
    {
        objectFile.Delete();
        e.print(std::cerr);
        return 1;
    }
    catch(const std::exception& e)
    {
        objectFile.Delete();
        return handleError(e);
    }

    return 0;
}
