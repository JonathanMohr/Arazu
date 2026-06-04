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
import copy
import shutil

def Build_Sources_To_Objects(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, src_dir: Path, build_dir: Path, doCompileCommands: bool) -> Path:
    patterns = ["*.c", "*.cpp"]

    files: list[Path] = []
    for pattern in patterns:
        files.extend(src_dir.rglob(pattern))

    objects: list[Path] = []
    for file in files:
        try:
            if file.suffix == ".c":
                object = toolchain.Compile_C_Source(mode, file, file.relative_to(src_dir), build_dir, doCompileCommands)
            elif file.suffix == ".cpp":
                object = toolchain.Compile_CPP_Source(mode, file, file.relative_to(src_dir), build_dir, doCompileCommands)
            else:
                logger.warning(f"Invalid source extension of file {file}")
                continue

        except Exception as e:
            logger.error(f"Compilation of {file} failed: {e}")
            raise e
        
        objects.append(object)

    return objects

def Build_Static_Library(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, src_dir: Path, build_dir: Path, name: str, doCompileCommands: bool) -> Path:
    objects = Build_Sources_To_Objects(logger, toolchain, mode, src_dir, build_dir, doCompileCommands)
    
    try:
        lib = toolchain.Archive_Objects(mode, objects, name, build_dir)

    except Exception as e:
        logger.error(f"Archiving static library {name} failed: {e}")
        raise e
    
    return lib

def Build_Dynamic_Library(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, libraries: list[Path], src_dir: Path, build_dir: Path, name: str, doCompileCommands: bool, plugin: bool = False) -> tuple[Path, Path | None, Path | None]:
    objects = Build_Sources_To_Objects(logger, toolchain, mode, src_dir, build_dir, doCompileCommands)
    
    try:
        dylib, implib, debug_info = toolchain.Link_DynamicLibrary(mode, objects, libraries, name, build_dir, plugin)

    except Exception as e:
        logger.error(f"Linking dynamic library {name} failed: {e}")
        raise e
    
    return dylib, implib, debug_info

# uses: target_os, target_arch, werror, portability, linking, assertions, sanitizers, host, sysroot
# kind of: optimization, uses it for release and release_with_debug_info, sets it for debug
# sets: lto, pic, hidden, debuginfo
def Build_Dist_Library(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, dll_libraries: list[Path], src_dir: Path, build_dir: Path, name: str) -> tuple[list[tuple[Path, Path | None, Path | None]], list[Path]]:

    dynamic_libs: list[tuple[Path, Path | None, Path | None]] = []
    static_libs: list[Path] = []

    # Release

    r_name = f"{name}"
    sr_name = f"{name}s"
    r_build_dir = build_dir / r_name

    r_mode = copy.deepcopy(mode)
    r_mode.lto = True
    r_mode.pic = True
    r_mode.hidden = True
    r_mode.debuginfo = False

    sr_mode = copy.deepcopy(mode)
    sr_mode.lto = False
    sr_mode.pic = False
    sr_mode.hidden = True
    sr_mode.debuginfo = False

    r_dynamic_lib, r_import_lib, r_debug_info = Build_Dynamic_Library(logger, toolchain, r_mode, dll_libraries, src_dir, r_build_dir / "dynamic", r_name, False)
    r_static_lib = Build_Static_Library(logger, toolchain, sr_mode, src_dir, r_build_dir / "static", sr_name, False)

    dynamic_libs.append((r_dynamic_lib, r_import_lib, r_debug_info))
    static_libs.append(r_static_lib)

    # Release with debug info

    rd_name = f"{name}rd"
    srd_name = f"{name}srd"
    rd_build_dir = build_dir / rd_name

    rd_mode = copy.deepcopy(mode)
    rd_mode.lto = True
    rd_mode.pic = True
    rd_mode.hidden = True
    rd_mode.debuginfo = True

    srd_mode = copy.deepcopy(mode)
    srd_mode.lto = False
    srd_mode.pic = False
    srd_mode.hidden = True
    srd_mode.debuginfo = True

    rd_dynamic_lib, rd_import_lib, rd_debug_info = Build_Dynamic_Library(logger, toolchain, rd_mode, dll_libraries, src_dir, rd_build_dir / "dynamic", rd_name, False)
    rd_static_lib = Build_Static_Library(logger, toolchain, srd_mode, src_dir, rd_build_dir / "static", srd_name, False)

    dynamic_libs.append((rd_dynamic_lib, rd_import_lib, rd_debug_info))
    static_libs.append(rd_static_lib)

    # Debug

    d_name = f"{name}d"
    sd_name = f"{name}sd"
    d_build_dir = build_dir / d_name

    d_mode = copy.deepcopy(mode)
    d_mode.optimization = OPTIMIZATION.NONE
    d_mode.lto = False
    d_mode.pic = True
    d_mode.hidden = True
    d_mode.debuginfo = True

    sd_mode = copy.deepcopy(mode)
    sd_mode.optimization = OPTIMIZATION.NONE
    sd_mode.lto = False
    sd_mode.pic = False
    sd_mode.hidden = True
    sd_mode.debuginfo = True

    d_dynamic_lib, d_import_lib, d_debug_info = Build_Dynamic_Library(logger, toolchain, d_mode, dll_libraries, src_dir, d_build_dir / "dynamic", d_name, True)
    d_static_lib = Build_Static_Library(logger, toolchain, sd_mode, src_dir, d_build_dir / "static", sd_name, False)

    dynamic_libs.append((d_dynamic_lib, d_import_lib, d_debug_info))
    static_libs.append(d_static_lib)

    return dynamic_libs, static_libs


def Copy_Path(logger: logging.Logger, src: Path, dst: Path):
    if src.exists():
        if dst.exists():
            if dst.is_file(): dst.unlink()
            elif dst.is_dir(): shutil.rmtree(str(dst))

        if src.is_file():
            shutil.copy2(str(src), str(dst))
        elif src.is_dir():
            shutil.copytree(str(src), str(dst))
    else:
        logger.warning(f"{src} does not exist")

def main() -> bool:
    dist_build = True

    argparser = argparse.ArgumentParser()

    # TODO

    args = argparser.parse_args()


    include_dir = Path("include")
    lib_dir = Path("libs")
    os_dir = Path("os")
    windows_dir = os_dir / "windows"

    dist_dir = Path("dist")
    general_build_dir = Path("build")
    general_log_dir = Path("logs")

    specific_build_dir = general_build_dir / ("dist_build" if dist_build else "local_build")
    specific_log_dir = general_log_dir / ("dist_build" if dist_build else "local_build")

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
        target_os=OS.macOS,
        target_arch=ARCH.arm64,
        werror=True,
        lto=True,
        pic=False,
        hidden=False,
        optimization=OPTIMIZATION.SPEED,
        portability=PORTABILITY.PORTABLE,
        linking=LINKING.DYNAMIC,
        assertions=False,
        sanitizers=False,
        debuginfo=False,
        host=HOST.FREESTANDING,
        sysroot=None
    )

    dll_libraries: list[Path] = []

    try:
        if dist_build:
            # werror = True
            # lto = True except static library
            # pic = True on dynamic library
            # hidden = True on library
            # optimization = SPEED or SIZE
            # portability = MACHINE or PORTABLE
            # linking = static or dynamic
            # assertions = off
            # sanitizers = off
            # debuginfo = False except specific library
            # host = specific per lib

            build_dir = specific_build_dir
            log_dir = specific_log_dir
        
            if buildMode.target_os == OS.Windows:
                windows_dll = Build_Static_Library(logger, toolchain, buildMode, windows_dir / "dll", build_dir / "windows", "windows_dll", True)
                dll_libraries.append(windows_dll)

            core_dynamic_libraries, core_static_libraries = Build_Dist_Library(logger, toolchain, buildMode, dll_libraries, lib_dir / "core", build_dir / "core", "arazu")

            if dist_dir.exists(): shutil.rmtree(str(dist_dir))

            dist_lib_dir = dist_dir / "lib"
            dist_lib_dir.mkdir(parents=True, exist_ok=True)
            for core_dynamic_library in core_dynamic_libraries:
                core_dylib, core_implib, core_debug_info = core_dynamic_library

                dst_dylib = dist_lib_dir / core_dylib.name
                Copy_Path(logger, core_dylib, dst_dylib)

                if core_implib is not None:
                    dst_implib = dist_lib_dir / core_implib.name
                    Copy_Path(logger, core_implib, dst_implib)

                if core_debug_info is not None:
                    dst_debug_info = dist_lib_dir / core_debug_info.name
                    Copy_Path(logger, core_debug_info, dst_debug_info)

            for core_static_library in core_static_libraries:
                dst_static_lib = dist_lib_dir / core_static_library.name
                Copy_Path(logger, core_static_library, dst_static_lib)
        else:
            pass

    except Exception as e:
        print(e)

        buildCache.save()
        return False

    compileCommands.write(compileCommandsPath)
    buildCache.save()

    return True

if not main():
    sys.exit(1)
