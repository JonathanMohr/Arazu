from ci.defs import OS, ARCH, BuildContext

from ci.toolchains.get import Get_LLVM_Toolchain

from ci.compileCommands import CompileCommands
import ci.cache as cacheModule
import ci.logger as loggerModule

from pathlib import Path
import sys
import argparse
import logging

def main(args) -> bool:
    general_build_dir = Path("build")
    general_log_dir = Path("logs")

    compileCommandsPath = Path("compile_commands.json")

    logger = logging.getLogger("ci")
    logger.setLevel(logging.DEBUG)

    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.DEBUG)
    console_formatter = logging.Formatter("[%(levelname)s] %(message)s")
    console_handler.setFormatter(console_formatter)
    logger.addHandler(console_handler)


    buildCache = cacheModule.BuildCache(general_build_dir / "cache.json", logger)
    compileCommands = CompileCommands()


    buildContext = BuildContext(logger, buildCache, compileCommands)
    toolchain = Get_LLVM_Toolchain(buildContext)

    dist_build = True
    if dist_build:
        pass
    else:
        pass

    compileCommands.write(compileCommandsPath)
    buildCache.save()

    return True

argparser = argparse.ArgumentParser()

# TODO: args

args = argparser.parse_args()

if not main(args):
    sys.exit(1)
