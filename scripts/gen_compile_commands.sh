#!/usr/bin/env bash

echo GENERATING compile_commands.json...

rm -rf ./emacs
rm ./compile_commands.json
cmake -S . -B emacs -G Ninja -DCMAKE_BUILD_TYPE=Debug -DEMACS=ON
cp ./emacs/compile_commands.json ./compile_commands.json
rm -rf ./emacs

echo FINISHED!
