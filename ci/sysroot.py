from ci.defs import OS, ARCH

from pathlib import Path
import argparse
import logging
import urllib.request, urllib
import shutil
import hashlib
import tarfile
import os as os_module

SYSROOT_PATH = Path("sysroots")
TMP_PATH = Path("tmp")

def getSysrootPath(os: OS, arch: ARCH):
    match os:
        case OS.Windows: target_os_str = "windows"
        case OS.macOS: target_os_str = "macos"
        case OS.Linux: target_os_str = "linux"

    match arch:
        case ARCH.x86_64: target_arch_str = "x86_64"
        case ARCH.arm64: target_arch_str = "arm64"

    return SYSROOT_PATH / target_os_str / target_arch_str

def make_link(base_dir: Path, name: str, link_target: str, is_symlink: bool):
    path = base_dir / name
    try:
        path.relative_to(base_dir.resolve())
    except ValueError:
        raise RuntimeError(f"unsafe path in archive: {name}")
    
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() or path.is_symlink():
        path.unlink()
    try:
        if is_symlink:
            os_module.symlink(link_target, path)
        else:
            os_module.link((base_dir / link_target).resolve(), path)
    except OSError:
        source = (path.parent / link_target).resolve()
        if source.is_dir():
            shutil.copytree(source, path, symlinks=False, dirs_exist_ok=True)
        elif source.exists():
            shutil.copy2(source, path)

def installGlibcLinuxSysroot(logger: logging.Logger, dest: Path, arch: ARCH):
    SHA256 = {
        ARCH.x86_64: "52d61d4446ffebfaa3dda2cd02da4ab4876ff237853f46d273e7f9b666652e1d",
        ARCH.arm64:  "c7176a4c7aacbf46bda58a029f39f79a68008d3dee6518f154dcf5161a5486d8",
    }

    sha256 = SHA256[arch]
    url = f"https://commondatastorage.googleapis.com/chrome-linux-sysroot/{sha256}"

    # Delete file
    tar_path = TMP_PATH / "sysroot.tar.xz"
    tar_path.parent.mkdir(parents=True, exist_ok=True)
    if tar_path.exists():
        if tar_path.is_dir():
            shutil.rmtree(str(tar_path))
        else:
            tar_path.unlink()

    # install archive
    with urllib.request.urlopen(url) as response, tar_path.open("wb") as f:
        shutil.copyfileobj(response, f)

    # Check archive
    h = hashlib.sha256()
    with tar_path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)

    if h.hexdigest() != sha256:
        raise RuntimeError("checksum mismatch: file is corrupt or wrong")
    
    # extract files and folders
    with tarfile.open(str(tar_path), "r:xz") as tar:
        for member in tar.getmembers():
            if member.issym() or member.islnk():
                continue
            target = (dest / member.name).resolve()
            try:
                target.relative_to(dest.resolve())
            except ValueError:
                raise RuntimeError(f"unsafe path in archive: {member.name}")
            tar.extract(member, dest, filter="data")

    # extract links
    with tarfile.open(str(tar_path), "r:xz") as tar:
        for member in tar.getmembers():
            if member.issym() or member.islnk():
                make_link(dest, member.name, member.linkname, member.issym())


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

    installGlibcLinuxSysroot(logger, getSysrootPath(OS.Linux, ARCH.x86_64), ARCH.x86_64)
