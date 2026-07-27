from ci.defs import OS, ARCH, OPTIMIZATION, PORTABILITY, LINKING, HOST, BuildContext, BuildMode, DynamicLibrary, StaticLibrary, Library

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
import platform

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
def Build_Dist_Library(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, dll_libraries: list[Path], src_dir: Path, build_dir: Path, name: str) -> Library:
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

    dynamicLibrary = DynamicLibrary((r_dynamic_lib, r_import_lib, r_debug_info), (d_dynamic_lib, d_import_lib, d_debug_info), (rd_dynamic_lib, rd_import_lib, rd_debug_info))
    staticLibrary = StaticLibrary(r_static_lib, d_static_lib, rd_static_lib)

    return Library(dynamicLibrary, staticLibrary)


def Build_Executable(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, static_libs: list[Path], src_dir: Path, build_dir: Path, name: str) -> tuple[Path, Path | None]:
    objects = Build_Sources_To_Objects(logger, toolchain, mode, src_dir, build_dir, True)
    
    libs: list[Path] = []

    for static_lib in static_libs:
        libs.append(static_lib)

    try:
        executable, executable_debug_info = toolchain.Link_Executable(mode, objects, libs, name, build_dir)

    except Exception as e:
        logger.error(f"Linking executable {name} failed: {e}")
        raise e
    
    return executable, executable_debug_info


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


def StageOther(logger: logging.Logger, dist_dir: Path):
    project_root = Path(".")

    readme = project_root / "README.md"
    license = project_root / "LICENSE"
    notice = project_root / "NOTICE"

    dist_license = dist_dir / "LICENSE"
    dist_notice = dist_dir / "NOTICE"

    dist_share = dist_dir / "share"

    dist_doc = dist_share / "doc" / "arazu"

    # .

    ## License
    dist_license.parent.mkdir(parents=True, exist_ok=True)
    Copy_Path(logger, license, dist_license)

    ## Notice
    dist_notice.parent.mkdir(parents=True, exist_ok=True)
    Copy_Path(logger, notice, dist_notice)

    # share/
    dist_share.mkdir(parents=True, exist_ok=True)

    ## doc/
    dist_doc.mkdir(parents=True, exist_ok=True)

    ### LICENSE
    dist_doc_license = dist_doc / "LICENSE"
    Copy_Path(logger, license, dist_doc_license)

    ### NOTICE
    dist_doc_notice = dist_doc / "NOTICE"
    Copy_Path(logger, notice, dist_doc_notice)

    ### README.md
    dist_doc_readme = dist_doc / "README.md"
    Copy_Path(logger, readme, dist_doc_readme)

def StageLibrariesPrepare(logger: logging.Logger, dist_dir: Path, include_path: Path, libraries: list[Library]) -> tuple[Path, Path]:
    include_dir = dist_dir / "include"
    lib_dir = dist_dir / "lib"

    include_dir.mkdir(parents=True, exist_ok=True)
    lib_dir.mkdir(parents=True, exist_ok=True)

    # Include directory
    Copy_Path(logger, include_path, include_dir)

    # Libraries
    for library in libraries:
        dynamic_lib = library.GetDynamic()
        static_lib = library.GetStatic()

        for dynamic_library in [
            (dynamic_lib.GetRelease(), dynamic_lib.GetReleaseImp(), dynamic_lib.GetReleaseInfo()),
            (dynamic_lib.GetDebug(), dynamic_lib.GetDebugImp(), dynamic_lib.GetDebugInfo()),
            (dynamic_lib.GetReleaseWithDebugInfo(), dynamic_lib.GetReleaseWithDebugInfoImp(), dynamic_lib.GetReleaseWithDebugInfoInfo())
        ]:
            dylib, implib, debug_info = dynamic_library

            dst_dylib = lib_dir / dylib.name
            Copy_Path(logger, dylib, dst_dylib)

            if implib is not None:
                dst_implib = lib_dir / implib.name
                Copy_Path(logger, implib, dst_implib)

            if debug_info is not None:
                dst_debug_info = lib_dir / debug_info.name
                Copy_Path(logger, debug_info, dst_debug_info)

        for static_library in [static_lib.GetRelease(), static_lib.GetDebug(), static_lib.GetReleaseWithDebugInfo()]:
            dst_static_library = lib_dir / static_library.name
            Copy_Path(logger, static_library, dst_static_library)

    return (include_dir, lib_dir)

def AddLibrary(logger: logging.Logger, lib_dir: Path, libraries: list[Library]):
    for library in libraries:
            dynamic_lib = library.GetDynamic()
            static_lib = library.GetStatic()
    
            for dynamic_library in [
                (dynamic_lib.GetRelease(), dynamic_lib.GetReleaseImp(), dynamic_lib.GetReleaseInfo()),
                (dynamic_lib.GetDebug(), dynamic_lib.GetDebugImp(), dynamic_lib.GetDebugInfo()),
                (dynamic_lib.GetReleaseWithDebugInfo(), dynamic_lib.GetReleaseWithDebugInfoImp(), dynamic_lib.GetReleaseWithDebugInfoInfo())
            ]:
                dylib, implib, debug_info = dynamic_library
    
                dst_dylib = lib_dir / dylib.name
                Copy_Path(logger, dylib, dst_dylib)
    
                if implib is not None:
                    dst_implib = lib_dir / implib.name
                    Copy_Path(logger, implib, dst_implib)
    
                if debug_info is not None:
                    dst_debug_info = lib_dir / debug_info.name
                    Copy_Path(logger, debug_info, dst_debug_info)
    
            for static_library in [static_lib.GetRelease(), static_lib.GetDebug(), static_lib.GetReleaseWithDebugInfo()]:
                dst_static_library = lib_dir / static_library.name
                Copy_Path(logger, static_library, dst_static_library)

def StageExecutables(logger: logging.Logger, dist_dir: Path, executables: list[tuple[Path, Path | None]]):
    bin_dir = dist_dir / "bin"

    bin_dir.mkdir(parents=True, exist_ok=True)

    # Executables
    for exe in executables:
        executable, executable_debug_info = exe

        dst_exe = bin_dir / executable.name
        Copy_Path(logger, executable, dst_exe)

        if executable_debug_info is not None:
            dst_debug_info = bin_dir / executable_debug_info.name
            Copy_Path(logger, executable_debug_info, dst_debug_info)


def main() -> bool:
    dist_build = True

    argparser = argparse.ArgumentParser()

    argparser.add_argument(
        "--os",
        dest="os",
        type=str,
        choices=[
            "windows", "macos", "linux"
        ],
        default=None,
        help="Set target os"
    )
    argparser.add_argument(
        "--arch",
        dest="arch",
        type=str,
        choices=[
            "x86-64", "x86_64", "x86",
            "arm64", "arm"
        ],
        default=None,
        help="Set target arch"
    )
    argparser.add_argument(
        "--sysroot",
        dest="sysroot",
        type=str,
        default=None,
        help="Set sysroot path"
    )

    args = argparser.parse_args()

    logger = logging.getLogger("ci")
    logger.setLevel(logging.DEBUG)

    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.DEBUG)
    console_formatter = logging.Formatter("[%(levelname)s] %(message)s")
    console_handler.setFormatter(console_formatter)
    logger.addHandler(console_handler)

    host_os = False
    host_arch = False

    match args.os:
        case "windows": target_os = OS.Windows
        case "macos": target_os = OS.macOS
        case "linux": target_os = OS.Linux

        case None | _:
            if args.os is not None:
                logger.warning("Invalid OS")
            
            os_uname = platform.system().lower()
            if (os_uname == "windows"): target_os = OS.Windows
            elif (os_uname == "darwin"): target_os = OS.macOS
            elif (os_uname == "linux"): target_os = OS.Linux
            else:
                logger.error("Invalid OS")
                return False

    match args.arch:
        case "x86-64" | "x86_64" | "x86":
            target_arch = ARCH.x86_64
        case "arm64" | "arm":
            target_arch = ARCH.arm64

        case None | _:
            if args.arch is not None:
                logger.warning("Invalid arch")
            
            cpu_arch = platform.machine().lower()
            if (cpu_arch in ["x86_64", "amd64"]): target_arch = ARCH.x86_64
            elif (cpu_arch in ["arm64", "aarch64"]): target_arch = ARCH.arm64
            else:
                logger.error("Invalid arch")
                return False

    isHost = host_os and host_arch

    include_dir = Path("include")
    lib_dir = Path("libs")
    tools_dir = Path("tools")

    os_dir = Path("os")
    windows_dir = os_dir / "windows"

    dist_dir = Path("dist")
    general_build_dir = Path("build")
    general_log_dir = Path("logs")

    specific_build_dir = general_build_dir / ("dist_build" if dist_build else "local_build")
    specific_log_dir = general_log_dir / ("dist_build" if dist_build else "local_build")

    compileCommandsPath = Path("compile_commands.json")


    buildCache = cacheModule.BuildCache(general_build_dir / "cache.json", logger)
    compileCommands = CompileCommands()


    buildContext = BuildContext(logger, buildCache, compileCommands)
    toolchain = Get_LLVM_Toolchain(buildContext)
    toolchain.Set_STDC("c99")
    toolchain.Set_STDCPP("c++98")

    if target_os == OS.Linux:
        toolchain.Add_Define("_POSIX_C_SOURCE", "200809L")

    dll_libraries: list[Path] = []
    
    sysroot_path: str | None = args.sysroot
    if sysroot_path:
        sysroot_path = str(Path(sysroot_path).resolve())

    match target_os:
        case OS.Windows: linking = LINKING.DYNAMIC
        case OS.Linux: linking = LINKING.DYNAMIC
        case OS.macOS: linking = LINKING.DYNAMIC
        case _: linking = LINKING.STATIC

    try:
        if dist_dir.exists(): shutil.rmtree(str(dist_dir))

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

            buildMode = BuildMode(
                target_os=target_os,
                target_arch=target_arch,
                werror=True, # set
                lto=True, # set
                pic=False, # set
                hidden=False, # set
                optimization=OPTIMIZATION.SPEED,
                portability=PORTABILITY.PORTABLE,
                linking=linking,
                assertions=False, # set
                sanitizers=False, # set
                debuginfo=False, # set
                host=HOST.FREESTANDING, # set
                sysroot=sysroot_path,
                project_root=str(include_dir.parent.resolve())
            )

            match buildMode.target_os:
                case OS.Windows: target_os_str = "windows"
                case OS.macOS: target_os_str = "macos"
                case OS.Linux: target_os_str = "linux"

            match buildMode.target_arch:
                case ARCH.x86_64: target_arch_str = "x86_64"
                case ARCH.arm64: target_arch_str = "arm64"

            match buildMode.optimization:
                case OPTIMIZATION.SPEED: optimization_str = "speed"
                case OPTIMIZATION.SIZE: optimization_str = "size"

            match buildMode.portability:
                case PORTABILITY.PORTABLE: portability_str = "portable"
                case PORTABILITY.MACHINE: portability_str = "machine"

            match buildMode.linking:
                case LINKING.STATIC: linking_str = "static"
                case LINKING.DYNAMIC: linking_str = "dynamic"

            build_dir = specific_build_dir / target_os_str / target_arch_str / optimization_str / portability_str / linking_str
            log_dir = specific_log_dir / target_os_str / target_arch_str / optimization_str / portability_str / linking_str

            if buildMode.target_os == OS.Windows:
                windows_dll = Build_Static_Library(logger, toolchain, buildMode, windows_dir / "dll", build_dir / "os" / "windows", "windows_dll", True)
                dll_libraries.append(windows_dll)

            toolchain.Add_Define("ARAZU_BUILD")
            toolchain.Add_Include_Directory(include_dir)


            coreToolchain = copy.copy(toolchain)
            coreToolchain.Add_Include_Directory(lib_dir / "core")

            coreBuildMode = copy.copy(buildMode)

            coreLibrary = Build_Dist_Library(logger, coreToolchain, coreBuildMode, dll_libraries, lib_dir / "core", build_dir / "libs" / "core", "arazu")

            dist_include_dir, dist_lib_dir = StageLibrariesPrepare(logger, dist_dir, include_dir, [coreLibrary])

            elfToolchain = copy.copy(toolchain)
            elfToolchain.Add_Include_Directory(lib_dir / "elf")
            elfToolchain.Add_Library_Directory(dist_lib_dir)
            elfToolchain.Add_Library("arazus", coreLibrary.GetStatic().GetRelease())

            elfBuildMode = copy.copy(buildMode)

            elfLibrary = Build_Dist_Library(logger, elfToolchain, elfBuildMode, dll_libraries, lib_dir / "elf", build_dir / "libs" / "elf", "arazuelf")

            AddLibrary(logger, dist_lib_dir, [elfLibrary])


            arasmToolchain = copy.copy(toolchain)
            arasmToolchain.Add_Include_Directory(tools_dir / "arasm")
            arasmToolchain.Add_Library_Directory(dist_lib_dir)
            arasmToolchain.Add_Library("arazus", coreLibrary.GetStatic().GetRelease())
            arasmToolchain.Add_Library("arazuelfs", elfLibrary.GetStatic().GetRelease())

            arasmBuildMode = copy.copy(buildMode)
            arasmBuildMode.host = HOST.HOSTED

            arasmExecutable = Build_Executable(logger, arasmToolchain, arasmBuildMode, [], tools_dir / "arasm", build_dir / "tools" / "arasm", "arasm")
            

            StageExecutables(logger, dist_dir, [arasmExecutable])

            StageOther(logger, dist_dir)

        else:
            pass

    except Exception as e:
        logger.error(f"Build failed: {e}")

        buildCache.save()
        return False

    compileCommands.write(compileCommandsPath)
    buildCache.save()

    return True

if not main():
    sys.exit(1)
