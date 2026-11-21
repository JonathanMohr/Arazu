from enum import Enum
from dataclasses import dataclass

from build.buildlogger import BuildLogger
from build.cache import BuildCache
from build.compileCommands import CompileCommands

class OS(Enum):
    Windows = "Windows"
    macOS = "macOS"
    Linux = "Linux"

class ARCH(Enum):
    x86_64 = "x86-64"
    arm64 = "ARM64"


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
    logger: BuildLogger
    buildCache: BuildCache
    compileCommands: CompileCommands
