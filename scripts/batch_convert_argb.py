# this script will ran all the images in ../assets/images
# through ../tools/argb_converter.exe

import os
import subprocess

images_path = "../assets/images"
argb_path = "../assets/argbs"
tool_path = "../tools/argb_converter.exe"


def main():

    print("starting script...")

    images = os.listdir(images_path)
    for image in images:
        result = subprocess.run(
            [os.path.abspath(tool_path), os.path.abspath(images_path) + "\\" + image],
            capture_output=True,
            cwd=os.path.abspath(argb_path),
            text=True,
        )

        print("argb_converter: ", result.stdout)


if __name__ == "__main__":
    main()
