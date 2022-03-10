import os
import re


file_list = [
    x
    for x in os.listdir(os.getcwd())
    if os.path.splitext(x)[1] in (".h", "hpp", ".cpp")
]


def convert(match):
    return match.group(1) + match.group(2).upper()


for file in file_list:
    print("processing {}...".format(file))
    old_file = open(file).read()
    new_file = open(file, "w")
    for word in old_file.split():
        converted_word = "".join(word.title() for word in word.split("_"))
        old_file = old_file.replace(word, converted_word)
    new_file.write(old_file)


print("Finished")
