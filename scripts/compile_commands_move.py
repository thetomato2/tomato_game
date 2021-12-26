#!/usr/bin/env python3

import os
from os import listdir
from os.path import isfile, join, getmtime
import shutil
import sys
from pathlib import Path

print("Starting post build script")

# Properties
target_dir = Path(sys.argv[1])

print("Target directory: " + target_dir.as_posix())

def copy_compile_commands():



def main():
    print("Copying data directory...")
    update_data("./data", (target_dir).as_posix())


if __name__ == "__main__":
    main()

print("Ending post build script")
