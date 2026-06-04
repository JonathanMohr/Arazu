from ci.defs import BuildContext, BuildMode

from typing import Callable
from pathlib import Path
import shutil

class Toolchain:
    # self, mode, src, src_rel, out_dir
    Compile_C_Source: Callable[["Toolchain", BuildMode, Path, Path, Path], Path]
    Compile_CPP_Source: Callable[["Toolchain", BuildMode, Path, Path, Path], Path]

    # self, mode, objects, name, out_dir
    Archive_Objects: Callable[["Toolchain", BuildMode, list[Path], str, Path], Path]

    # self, mode, objects, libraries, name, out_dir
    Link_Executable: Callable[["Toolchain", BuildMode, list[Path], list[Path], str, Path], Path]

    # self, mode, objects, libraries, name, out_dir, plugins -> dylib, implib | None
    Link_DynamicLibrary: Callable[["Toolchain", BuildMode, list[Path], list[Path], str, Path, bool], tuple[Path, Path | None]]

    def __init__(self, context: BuildContext):
        self.context = context

        self.defines: list[str, str] = []
        self.includeDirs: list[Path] = []
        self.libraryDirs: list[Path] = []
        self.libraries: list[str] = []

        self.stdc: str = "c99"
        self.stdcpp: str = "c++98"

    def Set_STDC(self, stdc: str):
        self.stdc = stdc

    def Set_STDCPP(self, stdcpp: str):
        self.stdcpp = stdcpp

    def Add_Define(self, name: str, value: str = ""):
        self.defines.append((name, value))

    def Add_Include_Directory(self, dir: Path):
        self.includeDirs.append(dir)

    def Add_Library_Directory(self, dir: Path):
        self.libraryDirs.append(dir)

    def Add_Library(self, lib: str):
        self.libraries.append(lib)

class ToolchainError(RuntimeError):
    pass

def Require_Tool(name: str) -> str:
    path = shutil.which(name)
    if not path:
        raise ToolchainError(f"Missing required tool: {name}")
    return path
