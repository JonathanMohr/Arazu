from ci.defs import OS, ARCH

from pathlib import Path
import argparse
import logging

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

def installGlibcLinuxSysroot(logger: logging.Logger, out: Path, arch: ARCH):
    # TODO
    pass

if __name__ == "__main__":
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

    args = argparser.parse_args()

    logger = logging.getLogger("sysroot")
    logger.setLevel(logging.DEBUG)

    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.DEBUG)
    console_formatter = logging.Formatter("[%(levelname)s] %(message)s")
    console_handler.setFormatter(console_formatter)
    logger.addHandler(console_handler)

    installGlibcLinuxSysroot(getSysrootPath(OS.Linux, ARCH.x86_64), ARCH.x86_64)
