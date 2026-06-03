from ci.defs import OS, ARCH

import sys
import argparse

def main(args) -> bool:
    dist_build = True

    if dist_build:
        pass

    else:
        pass

    return True

argparser = argparse.ArgumentParser()



args = argparser.parse_args()

if not main(args):
    sys.exit(1)
