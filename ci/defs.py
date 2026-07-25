from enum import Enum
from dataclasses import dataclass
from pathlib import Path
import logging

from ci.cache import BuildCache
from ci.compileCommands import CompileCommands

class OS(Enum):
    Windows = 1
    macOS = 2
    Linux = 3

class ARCH(Enum):
    x86_64 = 1
    arm64 = 2


class OPTIMIZATION(Enum):
    NONE = 1
    SPEED = 2
    SIZE = 3

class PORTABILITY(Enum):
    MACHINE = 1
    PORTABLE = 2

class LINKING(Enum):
    STATIC = 1
    DYNAMIC = 2


class HOST(Enum):
    HOSTED = 1
    FREESTANDING = 2

@dataclass()
class BuildMode:
    target_os: OS
    target_arch: ARCH

    werror: bool

    lto: bool

    pic: bool
    hidden: bool

    optimization: OPTIMIZATION
    portability: PORTABILITY
    linking: LINKING
    
    assertions: bool
    sanitizers: bool
    debuginfo: bool

    host: HOST

    sysroot: str | None

    project_root: str

@dataclass()
class BuildContext:
    logger: logging.Logger
    buildCache: BuildCache
    compileCommands: CompileCommands

class DynamicLibrary:
    def __init__(self, release: tuple[Path, Path | None, Path | None], debug: tuple[Path, Path | None, Path | None], releaseWithDebugInfo: tuple[Path, Path | None, Path | None]):
        releaseLib, releaseImplib, releaseDebugInfo = release
        self.releaseLib = releaseLib
        self.releaseImplib = releaseImplib
        self.releaseDebugInfo = releaseDebugInfo

        debugLib, debugImplib, debugDebugInfo = debug
        self.debugLib = debugLib
        self.debugImplib = debugImplib
        self.debugDebugInfo = debugDebugInfo

        releaseWithDebugInfoLib, releaseWithDebugInfoImplib, releaseWithDebugInfoDebugInfo = releaseWithDebugInfo
        self.releaseWithDebugInfoLib = releaseWithDebugInfoLib
        self.releaseWithDebugInfoImplib = releaseWithDebugInfoImplib
        self.releaseWithDebugInfoDebugInfo = releaseWithDebugInfoDebugInfo

    def GetRelease(self) -> Path:
        return self.releaseLib

    def GetReleaseImp(self) -> Path | None:
        return self.releaseImplib

    def GetReleaseInfo(self) -> Path | None:
        return self.releaseDebugInfo

    def GetDebug(self) -> Path:
        return self.debugLib

    def GetDebugImp(self) -> Path | None:
        return self.debugImplib

    def GetDebugInfo(self) -> Path | None:
        return self.debugDebugInfo

    def GetReleaseWithDebugInfo(self) -> Path:
        return self.releaseWithDebugInfoLib

    def GetReleaseWithDebugInfoImp(self) -> Path | None:
        return self.releaseWithDebugInfoImplib

    def GetReleaseWithDebugInfoInfo(self) -> Path | None:
        return self.releaseWithDebugInfoDebugInfo

class StaticLibrary:
    def __init__(self, release: Path, debug: Path, releaseWithDebugInfo: Path):
        self.release = release
        self.debug = debug
        self.releaseWithDebugInfo = releaseWithDebugInfo

    def GetRelease(self) -> Path:
        return self.release

    def GetDebug(self) -> Path:
        return self.debug

    def GetReleaseWithDebugInfo(self) -> Path:
        return self.releaseWithDebugInfo

class Library:
    def __init__(self, dynamic: DynamicLibrary, static: StaticLibrary):
        self.dynamic = dynamic
        self.static = static

    def GetDynamic(self) -> DynamicLibrary:
        return self.dynamic

    def GetStatic(self) -> StaticLibrary:
        return self.static
