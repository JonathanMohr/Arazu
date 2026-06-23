from ci.defs import OS, ARCH

from pathlib import Path

SYSROOT_PATH = Path("sysroots")

def getSysrootPath(os: OS, arch: ARCH):
    match os:
        case OS.Windows: target_os_str = "windows"
        case OS.macOS: target_os_str = "macos"
        case OS.Linux: target_os_str = "linux"

    match arch:
        case ARCH.x86_64: target_arch_str = "x86_64"
        case ARCH.arm64: target_arch_str = "arm64"

    return SYSROOT_PATH / target_os_str / target_arch_str

def installGlibcLinuxSysroot(out: Path, arch: ARCH):
    # TODO
    pass

if __name__ == "__main__":
    installGlibcLinuxSysroot(getSysrootPath(OS.Linux, ARCH.x86_64), ARCH.x86_64)
