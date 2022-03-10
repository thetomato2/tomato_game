import os
import re


file_list = [
    x
    for x in os.listdir(os.getcwd())
    if os.path.splitext(x)[1] in (".h", "hpp", ".cpp")
]

REG = r"(.*?)_([a-zA-Z])"


def to_camel(word):
    comps = word.split("_")
    return comps[0] + "".join(x.title() for x in comps[1:])


for file in file_list:
    print("processing {}...".format(file))
    old_file = open(file).read()
    for word in old_file.split():
        converted_word = to_camel(word)
        old_file = old_file.replace(word, converted_word)
    new_file = open(file, "w")
    new_file.write(old_file)


print("Finished")
