from build.defs import OS, ARCH, OPTIMIZATION, PORTABILITY, LINKING, HOST, BuildContext, BuildMode

from build.toolchain.toolchain import Toolchain
from build.toolchain.get import Get_LLVM_Toolchain

from build.compileCommands import CompileCommands
import build.cache as cacheModule
import build.buildlogger as buildlogger
import build.archive as archiveModule

from pathlib import Path
import sys
import argparse
import logging
import copy
import shutil
import platform
import subprocess

def Build_Sources_To_Objects(logger: logging.Logger, toolchain: Toolchain, mode: BuildMode, src_dir: Path, build_dir: Path, doCompileCommands: bool) -> list[Path]:
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


def run_git(cmd: list[str]):
    try:
        return subprocess.check_output(cmd, stderr=subprocess.DEVNULL).decode().strip()
    except subprocess.CalledProcessError:
        return None

def get_version() -> str:
    commit_hash = run_git(["git", "rev-parse", "--short", "HEAD"])
    if not commit_hash:
        commit_hash = "unknown"

    last_tag = run_git(["git", "describe", "--tags", "--abbrev=0"])
    
    if last_tag:
        commits_since_tag = run_git(["git", "rev-list", f"{last_tag}..HEAD", "--count"])
        if commits_since_tag and commits_since_tag != "0":
            version = f"{last_tag}-dev.{commits_since_tag}+{commit_hash}"
        else:
            version = f"{last_tag}+{commit_hash}"
    else:
        branch = run_git(["git", "rev-parse", "--abbrev-ref", "HEAD"]) or "unknown-branch"
        version = f"{branch}+{commit_hash}"

    return version


def main() -> bool:
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

    argparser.add_argument(
        "-v", "--version",
        dest="version",
        metavar="NAME",
        type=str,
        help="Version string"
    )

    argparser.add_argument(
        "-d",
        dest="debug",
        action="store_true",
        help="Build with debug mode"
    )


    argparser.add_argument(
        "-a", "--archive",
        dest="archive",
        action="store_true",
        help="Archive the projekt"
    )

    argparser.add_argument(
        "-A", "--archive-name",
        dest="archive_name",
        metavar="NAME",
        type=str,
        help="Name of the archive to create"
    )


    args = argparser.parse_args()

    logger = buildlogger.get("build")
    logger.setLevel(logging.DEBUG)

    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.DEBUG)
    console_formatter = logging.Formatter("[%(levelname)s] %(message)s")
    console_handler.setFormatter(console_formatter)
    logger.addHandler(console_handler)

    if args.version:
        version = args.version
    else:
        version = get_version()


    os_uname = platform.system().lower()
    if (os_uname == "windows"): host_os = OS.Windows
    elif (os_uname == "darwin"): host_os = OS.macOS
    elif (os_uname == "linux"): host_os = OS.Linux
    else:
        logger.error(f"Unknown host OS uname: {os_uname}")
        return False

    cpu_arch = platform.machine().lower()
    if (cpu_arch in ["x86_64", "amd64"]): host_arch = ARCH.x86_64
    elif (cpu_arch in ["arm64", "aarch64"]): host_arch = ARCH.arm64
    else:
        logger.error(f"Unknown host architecture: {cpu_arch}")
        return False

    match args.os:
        case "windows": target_os = OS.Windows
        case "macos": target_os = OS.macOS
        case "linux": target_os = OS.Linux

        case None | _:
            if args.os is not None:
                logger.error(f"Invalid OS: {args.os}")
                return False

            target_os = host_os

    match args.arch:
        case "x86-64" | "x86_64" | "x86":
            target_arch = ARCH.x86_64
        case "arm64" | "arm":
            target_arch = ARCH.arm64

        case None | _:
            if args.arch is not None:
                logger.error(f"Invalid arch: {args.arch}")
                return False
            
            target_arch = host_arch


    project_dir = Path(".")
    include_dir = project_dir / "include"
    # libs_dir = project_dir / "libs"
    tools_dir = project_dir / "tools"


    archives_path = project_dir / "archives"

    dist_dir = project_dir / "dist"
    general_build_dir = project_dir / ".build"
    general_log_dir = project_dir / "logs"

    compileCommandsPath = project_dir / "compile_commands.json"


    sysroot_path: str | None = args.sysroot
    if sysroot_path:
        sysroot_path = str(Path(sysroot_path).resolve())


    if args.debug:
        buildMode = BuildMode(
            target_os=target_os,
            target_arch=target_arch,
            werror=True,
            lto=False,
            pic=False,
            hidden=False,
            optimization=OPTIMIZATION.NONE,
            portability=PORTABILITY.PORTABLE,
            linking=LINKING.STATIC,
            assertions=True,
            sanitizers=True,
            debuginfo=True,
            host=HOST.FREESTANDING,
            sysroot=sysroot_path,
            project_root=str(project_dir.resolve())
        )
    else:
        buildMode = BuildMode(
            target_os=target_os,
            target_arch=target_arch,
            werror=True,
            lto=True,
            pic=False,
            hidden=False,
            optimization=OPTIMIZATION.SPEED,
            portability=PORTABILITY.PORTABLE,
            linking=LINKING.STATIC,
            assertions=False,
            sanitizers=False,
            debuginfo=False,
            host=HOST.FREESTANDING,
            sysroot=sysroot_path,
            project_root=str(project_dir.resolve())
        )

    build_dir = general_build_dir / ("debug" if args.debug else "release") / str(target_os) / str(target_arch) # / optimization_str / portability_str / linking_str
    _log_dir = general_log_dir / ("debug" if args.debug else "release") / str(target_os) / str(target_arch) # / optimization_str / portability_str / linking_str

    buildCache = cacheModule.BuildCache(general_build_dir / "cache.json", logger)
    compileCommands = CompileCommands()


    buildContext = BuildContext(logger, buildCache, compileCommands)
    toolchain = Get_LLVM_Toolchain(buildContext)
    toolchain.Set_STDC("c99")
    toolchain.Set_STDCPP("c++98")
    toolchain.Add_Define("ARAZU_BUILD")

    try:
        # version
        version_include = build_dir / "version-include"
        version_include.mkdir(parents=True, exist_ok=True)
        version_header = version_include / "version.h"

        with version_header.open("w") as f:
            c_version = version.replace("\"", "\\\"")
            f.write(
                "#ifndef GENERATED_VERSION_HEADER_H\n"
                "#define GENERATED_VERSION_HEADER_H\n"
                "\n"
                f"#define VERSION \"{c_version}\"\n"
                "\n"
                "#endif // GENERATED_VERSION_HEADER_H\n"
            )

        toolchain.Add_Include_Directory(version_include)

        # tools
        tools_build_dir = build_dir / "tools"

        ## ARASM
        arasm_toolchain = toolchain.Copy()
        arasm_toolchain.Add_Include_Directory(include_dir)
        arasm_toolchain.Add_Include_Directory(tools_dir / "arasm")

        arasm_build_mode = copy.deepcopy(buildMode)
        arasm_build_mode.host = HOST.HOSTED

        arasm = Build_Executable(logger, arasm_toolchain, arasm_build_mode, [], tools_dir / "arasm", tools_build_dir / "arasm", "arasm")


        # dist
        if dist_dir.exists():
            shutil.rmtree(str(dist_dir))
        dist_dir.mkdir(parents=True, exist_ok=True)

        ## README
        readme = project_dir / "README.md"
        dist_readme = dist_dir / "README.md"
        Copy_Path(logger, readme, dist_readme)

        ## LICENSE
        license = project_dir / "LICENSE"
        dist_license = dist_dir / "LICENSE"
        Copy_Path(logger, license, dist_license)

        ## docs
        docs = project_dir / "docs"
        dist_docs = dist_dir / "docs"
        Copy_Path(logger, docs, dist_docs)

        ## bin
        bin_dir = dist_dir / "bin"
        bin_dir.mkdir(parents=True, exist_ok=True)

        ### arasm
        arasm_executable, arasm_executable_debug_info = arasm
        bin_arasm = bin_dir / arasm_executable.name
        Copy_Path(logger, arasm_executable, bin_arasm)
        if arasm_executable_debug_info is not None:
                    bin_arasm_debug_info = bin_dir / arasm_executable_debug_info.name
                    Copy_Path(logger, arasm_executable_debug_info, bin_arasm_debug_info)

    except Exception as e:
        logger.error(f"Build failed: {e}")

        buildCache.save()
        return False

    if args.archive:
        archives_path.mkdir(parents=True, exist_ok=True)
        _archive = archiveModule.archive(target_os, target_arch, dist_dir, archives_path / (args.archive_name or "arazu"))

    compileCommands.write(compileCommandsPath)
    buildCache.save()

    return True

if not main():
    sys.exit(1)
