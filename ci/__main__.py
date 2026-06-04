from ci.defs import OS, ARCH, OPTIMIZATION, PORTABILITY, LINKING, HOST, BuildContext, BuildMode

from ci.toolchains.toolchain import Toolchain
from ci.toolchains.get import Get_LLVM_Toolchain

from ci.compileCommands import CompileCommands
import ci.cache as cacheModule
import ci.logger as loggerModule

from pathlib import Path
import sys
import argparse
import logging

def Build_Sources_To_Objects(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, src_dir: Path, build_dir: Path) -> Path:
    patterns = ["*.c", "*.cpp"]

    files: list[Path] = []
    for pattern in patterns:
        files.extend(src_dir.rglob(pattern))

    objects: list[Path] = []
    for file in files:
        try:
            if file.suffix == ".c":
                object = toolchain.Compile_C_Source(mode, file, file.relative_to(src_dir), build_dir)
            elif file.suffix == ".cpp":
                object = toolchain.Compile_CPP_Source(mode, file, file.relative_to(src_dir), build_dir)
            else:
                logger.warning(f"Invalid source extension of file {file}")
                continue

        except Exception as e:
            logger.error(f"Compilation of {file} failed")
            raise e
        
        objects.append(object)

    return objects

def Build_Static_Library(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, src_dir: Path, build_dir: Path, name: str) -> Path:
    objects = Build_Sources_To_Objects(logger, toolchain, mode, src_dir, build_dir)
    
    try:
        lib = toolchain.Archive_Objects(mode, objects, name, build_dir)

    except Exception as e:
        logger.error(f"Archiving static library {name} failed")
        raise e
    
    return lib

def Build_Dynamic_Library(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, libraries: list[Path], src_dir: Path, build_dir: Path, name: str, plugin: bool = False) -> tuple[Path, Path]:
    objects = Build_Sources_To_Objects(logger, toolchain, mode, src_dir, build_dir)
    
    try:
        dylib, implib = toolchain.Link_DynamicLibrary(mode, objects, libraries, name, build_dir, plugin)

    except Exception as e:
        logger.error(f"Linking dynamic library {name} failed")
        raise e
    
    return dylib, implib

def main() -> bool:
    argparser = argparse.ArgumentParser()

    # TODO

    args = argparser.parse_args()


    include_dir = Path("include")
    lib_dir = Path("libs")
    os_dir = Path("os")
    windows_dir = os_dir / "windows"

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

    toolchain.Add_Include_Directory(include_dir)

    buildMode = BuildMode(
        target_os=OS.Windows,
        target_arch=ARCH.x86_64,
        werror=True,
        lto=False,
        pic=True,
        hidden=True,
        optimization=OPTIMIZATION.SPEED,
        portability=PORTABILITY.PORTABLE,
        linking=LINKING.DYNAMIC,
        assertions=False,
        sanitizers=False,
        debuginfo=True,
        host=HOST.FREESTANDING,
        sysroot=None
    )

    dll_libraries: list[Path] = []

    if buildMode.target_os == OS.Windows:
        try:
            windows_dll = Build_Static_Library(logger, toolchain, buildMode, windows_dir / "dll", general_build_dir / "windows", "windows_dll")

        except Exception as e:
            buildCache.save()
            return False
        
        dll_libraries.append(windows_dll)

    try:
        dylib, implib = Build_Dynamic_Library(logger, toolchain, buildMode, dll_libraries, lib_dir / "core", general_build_dir / "core", "core")
    except Exception:
        buildCache.save()
        return False

    dist_build = True
    if dist_build:
        pass
    else:
        pass

    compileCommands.write(compileCommandsPath)
    buildCache.save()

    return True

if not main():
    sys.exit(1)
